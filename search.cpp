#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <atomic>
#include "moveGen.h"
#include "evaluation.h"
#include "search.h"
#include "misc.h"



using u64 = uint64_t;

std::atomic<bool> stopSearch(false);

const int TT_SIZE = 1 << 20; // ~1M entries 

struct TTEntry {
    u64 key;
    int depth;
    int score;
    TTFlag flag; // EXACT, ALPHA, BETA
    Move bestMove;
};

TTEntry tt[TT_SIZE];

int quiescence(Board& board, int alpha, int beta, Color side) {
    nodes++;

    int stand_pat = evaluate(board, side);

    if (stand_pat >= beta)
        return beta;
    if (stand_pat > alpha)
        alpha = stand_pat;

    std::vector<Move> moves;
    generateLegalCaptures(board, side, moves);

    for (auto& m : moves) {
        if (getPieceType(board, m.to) == NONE)
            continue;

        Undo u = makeMove(m, board, side);

        int score = -quiescence(
            board,
            -beta,
            -alpha,
            (side == WHITE ? BLACK : WHITE)
        );

        undoMove(m, board, side, u);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

int alphaBeta(Board& board, int depth, int alpha, int beta, Color side, int ply) {

    nodes++;
    int alphaOriginal = alpha;
    int betaOriginal = beta;

    

    if (depth == 0) {

        return quiescence(board, alpha, beta, side);

    }

    Move bestMove;
    bestMove.from = -1;
    bestMove.to = -1;


    u64 hash = board.hash;
    int index = hash & (TT_SIZE - 1);
    TTEntry& entry = tt[index];
    if (entry.key == hash && entry.depth >= depth) {
        if (entry.flag == EXACT)
            return entry.score;

        if (entry.flag == LOWERBOUND)
            alpha = std::max(alpha, entry.score);

        if (entry.flag == UPPERBOUND)
            beta = std::min(beta, entry.score);

        if (alpha >= beta)
            return entry.score;
    }


    std::vector<Move> moves;
    generateLegalMoves(board, side, moves);

    if (moves.empty()) {
        if (isKingInCheck(side, board))
            return -MATE + ply;
        else
            return 0; // stalemate
    }

    Color oppSide = (side == WHITE ? BLACK : WHITE);
    int value = -10000000;

    Move ttMove;
    ttMove.from = -1;

    if (entry.key == hash && entry.bestMove.from != -1)
        ttMove = entry.bestMove;

    if (ttMove.from != -1) {
        for (int i = 0; i < moves.size(); i++) {
            if (moves[i] == ttMove) {
                std::swap(moves[0], moves[i]);
                break;
            }
        }
    }

    // Precompute scores once
    std::vector<int> scores(moves.size());
    for (int i = 0; i < moves.size(); i++)
        scores[i] = scoreMove(board, moves[i], side, ply);

    // Incremental selection
    for (int i = 0; i < moves.size(); i++) {
        if (stopSearch) {
            logToFile("Stopsearch");
            return evaluate(board, side);
        }
    
        int bestIndex = i;
        for (int j = i+1; j < moves.size(); j++)
            if (scores[j] > scores[bestIndex])
                bestIndex = j;

        std::swap(moves[i], moves[bestIndex]);
        std::swap(scores[i], scores[bestIndex]);

        Move m = moves[i];
        bool isCapture = (getPieceType(board, m.to) != NONE);

        Undo u = makeMove(m, board, side);

        int score = -alphaBeta(board, depth-1, -beta, -alpha, oppSide, ply + 1);

        undoMove(m, board, side, u);

        
        if (score > value) {
            value = score;
            bestMove = m;
        }
        alpha = std::max(alpha, value);
        
      

        if (alpha >= beta) {

            // killer moves
            if (!isCapture) {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = m;
            }

            // history heuristic
            history[side][m.from][m.to] += depth * depth;
            history[side][m.from][m.to] = std::min(history[side][m.from][m.to], 1000000);

            break;
        }
        
    }


    entry.key = hash;
    entry.depth = depth;
    entry.score = value;

    if (value <= alphaOriginal)
        entry.flag = UPPERBOUND;
    else if (value >= betaOriginal)
        entry.flag = LOWERBOUND;
    else
        entry.flag = EXACT;

    entry.bestMove = bestMove; // optional but useful
        return value;
    }



Move findBestMove(Board& board, Color side, int depth) {
   
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(history, 0, sizeof(history));

    nodes = 0;
    
    std::vector<Move> moves;
    logToFile("generated Moves findBestMove"); 
    generateLegalMoves(board, side, moves);
   

    Move bestMove;
    bestMove.from = -1;
    bestMove.to = -1;
    int bestScore = (side == WHITE) ? -10000000 : 10000000;

    

    std::vector<int> scores(moves.size());
    for (int i = 0; i < moves.size(); i++)
        scores[i] = scoreMove(board, moves[i], side, 1);

    // selection sort or partial sort
    for (int i = 0; i < moves.size(); i++) {
        int bestIndex = i;
        for (int j = i+1; j < moves.size(); j++)
            if (scores[j] > scores[bestIndex])
                bestIndex = j;
        std::swap(moves[i], moves[bestIndex]);
        std::swap(scores[i], scores[bestIndex]);
    }

    bool found = false;

    int alpha = -10000000;
    int beta = 10000000;

    for (auto& m : moves) {


        


        Undo u = makeMove(m, board, side);


        

        int score = -alphaBeta(board, depth - 1, -beta, -alpha,
                              (side == WHITE) ? BLACK : WHITE, 1);
        

        std::string log = numToPos(m.from) + " -> " + numToPos(m.to) + " " + std::to_string(score);
        logToFile(log);
        std::cerr << log << std::endl;
        undoMove(m, board, side, u);

        if (side == WHITE)
            alpha = std::max(alpha, score);
        else
            beta = std::min(beta, score);
        

        if ((!found) ||
            (side == WHITE && score > bestScore) ||
            (side == BLACK && score < bestScore)) {
            bestScore = score;
            bestMove = m;
            found = true;
        }
    }

    if (!found) {
        // No legal move found
        std::cerr << "findBestMove: no legal moves found (returning invalid move)\n";
        logToFile("findBestMove: no legal moves found (returning invalid move)");

        Move none;
        none.from = -1;
        none.to   = -1;
        return none;
    }

    // Make the selected move on the actual board
    makeMove(bestMove, board, side);
    logToFile(std::to_string(nodes));
    return bestMove;
}

Move search(Board& board, Color side, int depth) {
    
    Move bestMove = findBestMove(board, side, depth);

    return bestMove;
}
