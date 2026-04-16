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
    u64 kingBB = (side == WHITE) ? WK : BK;
    int kingSq = lsb(kingBB);

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

        u64 pawnBB = (side == WHITE) ? WP : BP; 
        if (1ULL << sq & pawnBB) {

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
            case WP:   attackUnits += 1; break;
            case WN: attackUnits += 3; break;
            case WB: attackUnits += 3; break;
            case WR:   attackUnits += 5; break;
            case WQ:  attackUnits += 9; break;
            case BP:   attackUnits += 1; break;
            case BN: attackUnits += 3; break;
            case BB: attackUnits += 3; break;
            case BR:   attackUnits += 5; break;
            case BQ:  attackUnits += 9; break;
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

    

    
    

    // --- Material count ---
    int whiteMaterial = 0;
    int blackMaterial = 0;
    
    whiteMaterial += popcount(board.pieces[WP])   * PAWN_VALUE;
    whiteMaterial += popcount(board.pieces[WN]) * KNIGHT_VALUE;
    whiteMaterial += popcount(board.pieces[WB]) * BISHOP_VALUE;
    whiteMaterial += popcount(board.pieces[WR])   * ROOK_VALUE;
    whiteMaterial += popcount(board.pieces[WQ])  * QUEEN_VALUE;

    blackMaterial += popcount(board.pieces[BP])   * PAWN_VALUE;
    blackMaterial += popcount(board.pieces[BN]) * KNIGHT_VALUE;
    blackMaterial += popcount(board.pieces[BB]) * BISHOP_VALUE;
    blackMaterial += popcount(board.pieces[BR])   * ROOK_VALUE;
    blackMaterial += popcount(board.pieces[BQ])  * QUEEN_VALUE;

    score += whiteMaterial;
    score -= blackMaterial;


    //Phase & King Safety

    int ksWhite = kingSafety(board, WHITE);
    int ksBlack = kingSafety(board, BLACK);


    int phase = std::min((whiteMaterial + blackMaterial), 4000);
    score += (ksWhite - ksBlack) * phase / 20000;


    return (side == WHITE) ? score : -score;
}



