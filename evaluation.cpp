#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdlib>
#include "evaluation.h"
#include "makeMove.h"
#include "constants.h"
#include "moveGen.h"
#include "attack.h"




using u64 = uint64_t;


int evaluate(const Board& board, Color side, std::vector<Move>& moves) {
    int score = 0;
    Board boardCopy = board;

    // === Generate all legal moves for the current side ===
    moves.clear();
    generateLegalMoves(boardCopy, side, moves);

    // === Checkmate and stalemate detection ===
    if (moves.empty()) {
        if (isKingInCheck(side, board)) {
            // Checkmate for the current side
            return (side == WHITE) ? -1000000 : 1000000;
        } else {
            // Stalemate 
            return 0; 
        }
    }

    Color oppSide = (side == WHITE) ? BLACK : WHITE;
    // Generate all legal moves for the opposite side 
    moves.clear();

    
    generateLegalMoves(boardCopy, oppSide, moves);

    

    if (moves.empty()) {
        if (isKingInCheck(oppSide, board)) {
            // Checkmate for the current side
            return (side == WHITE) ? 1000000 : -1000000;
        } else {
            // Stalemate
            return 0.5;
        }
    }

    //PST
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (ownPieces) {
        
        int pieceSq = lsb(ownPieces);
        ownPieces &= ownPieces - 1;

        Piece piece = getPieceType(board, pieceSq);

        if (side == WHITE) {
            switch (piece)
            {
                case PAWN:
                    score += (side == WHITE) ? PST_PAWN[pieceSq] : -PST_PAWN[63 - pieceSq];
                    break;

                case KNIGHT:
                    score += (side == WHITE) ? PST_KNIGHT[pieceSq] : -PST_KNIGHT[63 - pieceSq];
                    break;

                case KING:
                    score += (side == WHITE) ? PST_KING[pieceSq] : -PST_KING[63 - pieceSq];
                    break;

                case BISHOP:
                    score += PST_BISHOP[pieceSq];
                    break;

                case ROOK:
                    score += PST_ROOK[pieceSq];
                    break;

                case QUEEN:
                    score += PST_QUEEN[pieceSq];
                    break;
            }
        }
    }
    

    // --- Material count ---
    score += popcount(board.pawns_white)   * PAWN_VALUE;
    score += popcount(board.knights_white) * KNIGHT_VALUE;
    score += popcount(board.bishops_white) * BISHOP_VALUE;
    score += popcount(board.rooks_white)   * ROOK_VALUE;
    score += popcount(board.queens_white)  * QUEEN_VALUE;

    score -= popcount(board.pawns_black)   * PAWN_VALUE;
    score -= popcount(board.knights_black) * KNIGHT_VALUE;
    score -= popcount(board.bishops_black) * BISHOP_VALUE;
    score -= popcount(board.rooks_black)   * ROOK_VALUE;
    score -= popcount(board.queens_black)  * QUEEN_VALUE;

    // === slight bonus/penalty for being in check ===
    if (isKingInCheck(side, board)) {
        score += (side == WHITE) ? -50 : 50;
    }

    return score;
}