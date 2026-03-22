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
#include "misc.h"




using u64 = uint64_t;


int evaluate(const Board& board, Color side) {
    int score = 0;
    Board boardCopy = board;

    
    u64 all = board.all_white | board.all_black;

    //PST
    

    while (all) {
        
        int pieceSq = lsb(all);
        all &= all - 1;

        Piece piece = getPieceType(board, pieceSq);

        Color side = (getColor(board, pieceSq));

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
                score += (side == WHITE) ? PST_BISHOP[pieceSq] : -PST_BISHOP[63 - pieceSq];
                break;

            case ROOK:
                score += (side == WHITE) ? PST_ROOK[pieceSq] : -PST_ROOK[63 - pieceSq];
                break;

            case QUEEN:
                score += (side == WHITE) ? PST_QUEEN[pieceSq] : -PST_QUEEN[63 - pieceSq];
                break;
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
    if (isKingInCheck(WHITE, board)) {
        score -= 50;
    } else if (isKingInCheck(BLACK, board)) {
        score += 50;
    }

    return (side == WHITE) ? score : -score;
}

int scoreMove(const Board& board, Move& m, Color side, int ply) {
    Piece victim = getPieceType(board, m.to);
    Piece attacker = getPieceType(board, m.from);

    // 1. Promotions (VERY high priority)
    if (m.promotion != NONE) {
        return 100000 + pieceValue(m.promotion);
    }

    // 2. Captures (MVV-LVA)
    if (victim != NONE) {
        return 10000 + 10 * pieceValue(victim) - pieceValue(attacker);
    }

    // 3. Killer moves
    if (m == killerMoves[ply][0]) return 9000;
    if (m == killerMoves[ply][1]) return 8000;

    // 4. History heuristic
    return history[side][m.from][m.to];
}


