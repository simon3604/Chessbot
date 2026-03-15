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


int alphaBeta(Board& board, int depth, int alpha, int beta, Color side, std::vector<Move> moves) {

    if (stopSearch) {
        return 0;
    }

    generateLegalMoves(board, side, moves);
    if (depth == 0) {
        return evaluate(board, side, moves);
    }

    if (side == WHITE) {
        int value = -10000000;

        for (auto& m : moves) {
            Undo u = makeMove(m, board, side);

            if (!isKingInCheck(side, board)) {

                int score = alphaBeta(board, depth - 1, alpha, beta, BLACK, moves);
                value = std::max(value, score);
                alpha = std::max(alpha, score);
            }

            undoMove(m, board, side, u);
            if (alpha >= beta) break;
        }

        return value;
    }
    else {
        int value = 10000000;

        for (auto& m : moves) {
            Undo u = makeMove(m, board, side);

            if (!isKingInCheck(side, board)) {
                
                int score = alphaBeta(board, depth - 1, alpha, beta, WHITE, moves);
                value = std::min(value, score);
                beta = std::min(beta, score);
            }

            undoMove(m, board, side, u);
            if (alpha >= beta) break;
        }

        return value;
    }
}



Move findBestMove(Board& board, Color side, int depth) {
    std::vector<Move> moves;
   
    logToFile("generated Moves findBestMove"); 
    generateLegalMoves(board, side, moves);
    

    Move bestMove;
    bestMove.from = -1;
    bestMove.to = -1;
    int bestScore = (side == WHITE) ? -10000000 : 10000000;

    // --- Simple move ordering: captures first ---
    std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b){
        int av = pieceValueAtSq(board, a.to);
        int bv = pieceValueAtSq(board, b.to);
        if (av != bv) return av > bv;
        if (a.from != b.from) return a.from < b.from;
        return a.to < b.to;
    });

    
    

    bool found = false;

    for (auto& m : moves) {
        Undo undo = makeMove(m, board, side);
        if (undo.to == -1) { 
            undoMove(m, board, side, undo);
            continue;
        }

    

        int score = alphaBeta(board, depth, -1000000, 1000000,
                              (side == WHITE) ? BLACK : WHITE, moves);
        std::string log = numToPos(undo.from) + " " +
                  numToPos(undo.to) + ": " +
                  std::to_string(score);

        logToFile(log);
        undoMove(m, board, side, undo);

        

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

    return bestMove;
}

