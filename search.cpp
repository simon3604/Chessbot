#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <chrono>
#include <atomic>
#include <cassert>
#include "moveGen.h"
#include "evaluation.h"
#include "search.h"
#include "misc.h"
#include "movePicker.h"

using u64 = uint64_t;
using u32 = uint32_t;

const int TT_SIZE = 1 << 20; // ~1M entries

struct TTEntry
{
    u64 key;
    int depth;
    int score;
    TTFlag flag; // EXACT, ALPHA, BETA
    u32 bestMove;
};

TTEntry tt[TT_SIZE];

int quiescence(Board &board, int alpha, int beta, Color side, int ply)
{

    nodes++;

    int stand_pat = evaluate(board, side);

    if (stand_pat >= beta)
        return beta;
    if (stand_pat > alpha)
        alpha = stand_pat;

    u32 *moves = moveStack[ply];
    int moveCount = generateCaptures(board, side, moves, "quiescence");

    for (int i = 0; i < moveCount; i++)
    {

        Board before = board;
        Undo u = makeMove(moves[i], board, "quiescence");

        if (isKingInCheck(side, board, "quiescence"))
        {
            undoMove(moves[i], board, u, "quiescence");
            
            continue;
        }

        int score = -quiescence(
            board,
            -beta,
            -alpha,
            (side == WHITE) ? BLACK : WHITE, ply + 1);

        undoMove(moves[i], board, u, "quiescence");

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

int alphaBeta(Board &board, int depth, int alpha, int beta, Color side, int ply, int time, std::chrono::steady_clock::time_point start_time, bool nullAllowed)
{

    nodes++;

    if ((nodes & 1023) == 0)
    {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed >= std::chrono::milliseconds(time))
        {
            stopSearch = true;
            return alpha;
        }
    }

    int alphaOriginal = alpha;
    int betaOriginal = beta;

    if (ply >= MAX_PLY)
    {
        std::cerr << "PLY OVERFLOW\n";
        exit(1);
    }

    u64 hash = board.hash;
    int index = hash & (TT_SIZE - 1);

    TTEntry &entry = tt[index];

    if (depth == 0)
    {
        if (entry.key == hash && entry.depth >= depth)
        {
            return entry.score;
        }
        return quiescence(board, alpha, beta, side, ply);
    }
    u32 bestMove;

    if (entry.key == hash && entry.depth >= depth)
    {
        if (entry.flag == EXACT)
        {
            return entry.score;
        }
        if (entry.flag == LOWERBOUND && entry.score >= beta)
        {
            return entry.score;
        }
        if (entry.flag == UPPERBOUND && entry.score <= alpha)
        {
            return entry.score;
        }
    }

    // Null Move Pruning
    if (depth >= 3 && !isKingInCheck(side, board, "Null Move Pruning") && has_non_pawn_material(board, side) && nullAllowed)
    {
        Undo u = make_null_move(board);

        int R = 2 + depth / 4; // reduction
        GoParams go = {};
        int score = -alphaBeta(board, depth - 1 - R, -beta, -beta + 1, (side == WHITE) ? BLACK : WHITE, ply + 1, time, start_time, false);

        undo_null_move(board, u);

        if (score >= beta)
        {
            return beta;
        }
    }

    u32 *moves = moveStack[ply];

    Color oppSide = (side == WHITE ? BLACK : WHITE);
    int value = -10000000;

    u32 ttMove;
    ttMove = 0;

    if (entry.key == hash && entry.bestMove != 0)
    {
        ttMove = entry.bestMove;
    }

    MovePicker mp(board, ttMove, ply);
    int moveNumber = 0;

    while (true)
    {
        u32 m = mp.nextMove();
        u32 copy = m;

        if (m == 0)
            break;
        moveNumber++;

        if (stopSearch)
        {
            std::cerr << "Search stopped\n";
            return evaluate(board, side);
        }

        bool notCheck = !isKingInCheck(side, board, "alphaBeta");

        int reduction = 0;

        // Apply LMR only in safe conditions
        if (moveFlags(m) == QUIET && notCheck && depth >= 4 && moveNumber >= 6)
        {
            reduction = 1;

            if (depth >= 6 && moveNumber >= 12)
                reduction = 2;
        }

        Color expectedSide = side;

        // Board before = board;
        Undo u = makeMove(copy, board, "ab");

        int score;

        if (reduction > 0)
        {
            score = -alphaBeta(board, depth - 1 - reduction,
                               -alpha - 1, -alpha,
                               oppSide, ply + 1, time, start_time, true);

            if (score > alpha)
            {
                score = -alphaBeta(board, depth - 1,
                                   -beta, -alpha,
                                   oppSide, ply + 1, time, start_time, true);
            }
        }
        else
        {
            score = -alphaBeta(board, depth - 1,
                               -beta, -alpha,
                               oppSide, ply + 1, time, start_time, true);
        }
        undoMove(copy, board, u, "ab");

        if (score > value)
        {
            value = score;
            bestMove = m;
        }
        alpha = std::max(alpha, value);

        if (alpha >= beta)
        {

            // killer moves
            if (moveFlags(m) != CAPTURE)
            {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = m;
            }

            // history heuristic
            history[side][fromSquare(m)][toSquare(m)] += depth * depth;
            history[side][fromSquare(m)][toSquare(m)] = std::min(history[side][fromSquare(m)][toSquare(m)], 1000000);

            break;
        }
        if (stopSearch)
        {
            return score;
        }
    }

    if (value == -10000000)
    { // no moves found
        if (isKingInCheck(side, board, "alphaBeta2"))
            return -MATE + ply;
        else
            return 0;
    }

    int storeScore = value;

    if (value > MATE - 1)
        storeScore += ply;
    if (value < -MATE + 1)
        storeScore -= ply;

    if (value <= alphaOriginal)
    {
        entry.flag = UPPERBOUND;
    }
    else if (value >= betaOriginal)
    {
        entry.flag = LOWERBOUND;
    }
    else
    {
        entry.flag = EXACT;
    }

    if (entry.key != hash || depth >= entry.depth)
    {
        entry.key = hash;
        entry.depth = depth;
        entry.score = storeScore;

        if (value <= alphaOriginal)
            entry.flag = UPPERBOUND;
        else if (value >= betaOriginal)
            entry.flag = LOWERBOUND;
        else
            entry.flag = EXACT;

        entry.bestMove = bestMove;
    }

    return value;
}

u32 findBestMove(Board &board, int depth, int time, std::chrono::steady_clock::time_point start_time)
{

    for (int p = 0; p < MAX_PLY; p++)
    {
        for (int k = 0; k < 2; k++)
        {
            killerMoves[p][k] = 0;
        }
    }
    memset(history, 0, sizeof(history));

    nodes = 0;

    Color side = board.sideToMove;

    u32 *moves = moveStack[0];
    int moveCount = generatePseudoLegalMoves(board, board.sideToMove, moves);
    std::cerr << "Pseudolegal moves: " << moveCount << std::endl;

    u32 bestMove;
    u32 currentBestMove;

    int bestScore = -10000000;

    bool found = false;

    int alpha = -10000000;
    int beta = 10000000;

    u64 hash = board.hash;
    int index = hash & (TT_SIZE - 1);
    TTEntry &entry = tt[index];
    if (entry.key != hash)
    {
        entry.bestMove = 0;
    }

    u32 ttMove = 0;

    if (entry.key == hash &&
        fromSquare(entry.bestMove) != -1 &&
        fromSquare(entry.bestMove) != toSquare(entry.bestMove))
    {
        ttMove = entry.bestMove;
    }

    MovePicker mp(board, ttMove, 0);

    while (true)
    {
        u32 m = mp.nextMove();
        u32 copy = m;
        if (m == 0ULL)
        {
            break;
        }
        std::cerr << numToPos(fromSquare(m)) << numToPos(toSquare(m)) << std::endl;
        Color expectedSide = side;
        Board before = board;
        Undo u = makeMove(copy, board, "findbest");
        if (isKingInCheck(expectedSide, board, "findbestmove"))
        {
            undoMove(copy, board, u, "findbest");
          
            continue;
        }

        int score = -alphaBeta(board, depth - 1, -beta, -alpha,
                               (side == WHITE) ? BLACK : WHITE, 1, time, start_time, true);

        undoMove(copy, board, u, "findbest");
        

        alpha = std::max(alpha, score);


        if (!found || score > bestScore)
        {
            bestScore = score;
            bestMove = m;
            found = true;
        }
        if (stopSearch)
        {
            break;
        }
    }

    if (!found)
    {
        u32 none = 0;
        return none;
    }

    return bestMove;
}

u32 search(Board &board, GoParams go)
{
    stopSearch = false;

    u32 bestMove = 0;
    int timeToMove;

    int timeLeft = (board.sideToMove == WHITE) ? go.wtime : go.btime;
    int increment = (board.sideToMove == WHITE) ? go.winc : go.binc;
    // Use 2.25% of the time + half of the increment
    std::cerr << "Movetime: " << go.movetime;
    if (go.wtime != -1 || go.btime != -1)
    {
        timeToMove = timeLeft / 40 + (increment / 2);

        // If the increment puts us above the total time left
        // use the timeleft - 0.5 seconds
        if (timeToMove >= timeLeft)
        {
            timeToMove = timeLeft - 500;
        }
        // If 0.5 seconds puts us below 0
        // use 0.1 seconds to at least get some move.
        if (timeToMove < 0)
        {
            timeToMove = 100;
        }
    }
    else if (go.infinite == true || go.movetime == -1)
    {
        timeToMove = 10000000;
    }
    else
    {
        timeToMove = go.movetime;
    }

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    int maxDepth = 15;
    if (go.depth != -1 || go.depth > 0)
    {
        std::cerr << "depth" << go.depth;
        maxDepth = go.depth;
    }

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        auto start_time = std::chrono::steady_clock::now();
        u32 move = findBestMove(board, depth, timeToMove, start_time);
        std::cerr << "Depth " << depth << " searched" << std::endl;
        printBoardAsLetters(board, false);
        if (stopSearch)
            break;

        if (move == 0)
        {
            if (bestMove == 0)
            {
                std::cerr << "No legal moves found. Stalemate or checkmate." << std::endl;
                printBoardAsLetters(board, false);
                return 0;
            }
            break;
        }

        bestMove = move;
    }

    makeMove(bestMove, board, "search");

    return bestMove;
}
