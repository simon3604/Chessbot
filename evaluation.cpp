#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "evaluation.h"
#include "makeMove.h"
#include "constants.h"
#include "moveGen.h"
#include "attack.h"
#include "misc.h"




using u64 = uint64_t;

int kingSafety(const Board& board, Color side) {
    Color oppSide = (side == WHITE) ? BLACK : WHITE;
    int kingSq = (side == WHITE) ? lsb(board.king_white) :  lsb(board.king_black);

    int score = 0;

    // 1. Pawn Shield (weighted)
    int shieldScore = 0;
    int dir = (side == WHITE) ? 8 : -8;

    int file = fileOf(kingSq);
    int rank = kingSq / 8;

    for (int df = -1; df <= 1; df++) {
        int f = file + df;
        if (f < 0 || f > 7) continue;

        int r = rank + (side == WHITE ? 1 : -1);
        if (r < 0 || r > 7) continue;

        int sq = r * 8 + f;
        if (sq < 0 || sq >= 64) continue;

        if (getPieceType(board, sq) == PAWN &&
            getColor(board, sq) == side) {

            shieldScore += 15;

            int rank = sq / 8;
            if ((side == WHITE && rank >= 2) ||
                (side == BLACK && rank <= 5))
                shieldScore += 5;

        } else {
            shieldScore -= 10;
        }
    }

    score += shieldScore;

    // 2. Open / Semi-open files
    int kingFile = fileOf(kingSq);

    for (int f = kingFile - 1; f <= kingFile + 1; f++) {
        if (f < 0 || f > 7) continue;
        if (!hasPawnOnFile(board, f, side)) {
            if (!hasPawnOnFile(board, f, oppSide))
                score -= 15; // fully open file
            else
                score -= 10; // semi-open
        }
    }

    // 3. King Zone definition
    std::vector<int> kingZone;

    u64 zone = kingAttacks[kingSq];

    while (zone) {
        int sq = lsb(zone);
        zone &= zone - 1;
        kingZone.push_back(sq);
    }

    // extend forward 
    
    u64 kAtt = kingAttacks[kingSq];
    while (kAtt) {
        int sq = lsb(kAtt);
        kAtt &= kAtt - 1;

        int forward = sq + dir;
        if (forward >= 0 && forward < 64)
            kingZone.push_back(forward);
    }
    

    // 4. Attack units system
    int attackUnits = 0;
    
    u64 allAttackers = 0;

    for (int sq : kingZone) {
        allAttackers |= attackers(board, oppSide, sq);
    }

    int attackerCount = popcount(allAttackers);

    while (allAttackers) {
        int attackerSq = lsb(allAttackers);
        allAttackers &= allAttackers - 1;

        Piece attackerType = getPieceType(board, attackerSq);

        switch (attackerType) {
            case PAWN:   attackUnits += 1; break;
            case KNIGHT: attackUnits += 3; break;
            case BISHOP: attackUnits += 3; break;
            case ROOK:   attackUnits += 5; break;
            case QUEEN:  attackUnits += 9; break;
        }
    }

    // 5. Nonlinear scaling
    if (attackerCount > 1) {
        int danger = attackUnits * 2; // quadratic scaling
        score -= danger;
    }

    // 6. Direct checks (very important)
    if (isKingInCheck(side, board)) {
        score -= 50;
    }

    // 7. Weak squares around king
   
    u64 kAtt2 = kingAttacks[kingSq];
    while (kAtt2) {
        int sq = lsb(kAtt2);
        kAtt2 &= kAtt2 - 1;

        if (!isSquareAttacked(board, side, sq) &&
            isSquareAttacked(board, oppSide, sq)) {
            score -= 5;
        }
    }

    return score;
}

int evaluate(const Board& board, Color side) {
    int score = 0;
    Board boardCopy = board;
    u64 all = board.all_white | board.all_black;

    
    int pstScore = 0;

    //PST
    while (all) {
        int pieceSq = lsb(all);
        all &= all - 1;

        Piece piece = getPieceType(board, pieceSq);

        Color pieceColor = (getColor(board, pieceSq));

        switch (piece)
        {
            case PAWN:
                pstScore += (pieceColor == WHITE) ? PST_PAWN[pieceSq] : -PST_PAWN[63 - pieceSq];
                break;

            case KNIGHT:
                pstScore += (pieceColor == WHITE) ? PST_KNIGHT[pieceSq] : -PST_KNIGHT[63 - pieceSq];
                break;

            case KING:
                pstScore += (pieceColor == WHITE) ? PST_KING[pieceSq] : -PST_KING[63 - pieceSq];
                break;

            case BISHOP:
                pstScore += (pieceColor == WHITE) ? PST_BISHOP[pieceSq] : -PST_BISHOP[63 - pieceSq];
                break;

            case ROOK:
                pstScore += (pieceColor == WHITE) ? PST_ROOK[pieceSq] : -PST_ROOK[63 - pieceSq];
                break;

            case QUEEN:
                pstScore += (pieceColor == WHITE) ? PST_QUEEN[pieceSq] : -PST_QUEEN[63 - pieceSq];
                break;
        }
        
    
    }
    score += pstScore;
    

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


    //Phase & King Safety

    int ksWhite = kingSafety(board, WHITE);
    int ksBlack = kingSafety(board, BLACK);


    int material =
        popcount(board.queens_white | board.queens_black) * 900 +
        popcount(board.rooks_white | board.rooks_black) * 500 +
        popcount(board.knights_white | board.knights_black) * 300 +
        popcount(board.bishops_white | board.bishops_black) * 300;

    int phase = std::min(material, 4000);
    score += (ksWhite - ksBlack) * phase / 20000;


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


