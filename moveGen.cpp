#include <iostream>
#include <cstdint>
#include <vector>
#include <cstdlib>
#include "constants.h"
#include "moveGen.h"
#include "attack.h"
#include "makeMove.h"


using u64 = uint64_t;


inline int lsb(u64 bb) { return __builtin_ctzll(bb); }
inline int msb(u64 bb) { return 63 - __builtin_clzll(bb); }

inline int popcount(u64 x) { return __builtin_popcountll(x); }


inline Move mkMove(int from, int to, int from2, int to2, Piece piece, Piece captured, Piece promotion, u_int8_t flags)
{
    Move m;
    m.from = from;
    m.to = to;
    m.from2 = from2;
    m.to2 = to2;
    m.piece = piece;
    m.captured = captured;
    m.promotion = promotion;
    m.flags = flags;

    if (piece == NONE) {
        std::cout << "ERROR: mkMove created NONE piece!\n";
        std::cout << "from=" << from << " to=" << to << "\n";
        std::cout << "flags=" << (int)flags << "\n";
        exit(1);
    }
    return m;
}

u64 setOccupancy(int index, int bits, u64 mask) {
    u64 occupancy = 0ULL;
    for (int i = 0; i < bits; i++) {
        int square = __builtin_ctzll(mask);
        mask &= mask - 1;
        if (index & (1 << i)) occupancy |= (1ULL << square);
    }
    return occupancy;
}


u64 getRookAttacks(int sq, u64 occ)
{
    u64 attacks = 0ULL;
    int rank = sq / 8, file = sq % 8;
    for (int r = rank + 1; r <= 7; r++)
    {
        attacks |= 1ULL << (r * 8 + file);
        if (occ & (1ULL << (r * 8 + file)))
            break;
    }
    for (int r = rank - 1; r >= 0; r--)
    {
        attacks |= 1ULL << (r * 8 + file);
        if (occ & (1ULL << (r * 8 + file)))
            break;
    }
    for (int f = file + 1; f <= 7; f++)
    {
        attacks |= 1ULL << (rank * 8 + f);
        if (occ & (1ULL << (rank * 8 + f)))
            break;
    }
    for (int f = file - 1; f >= 0; f--)
    {
        attacks |= 1ULL << (rank * 8 + f);
        if (occ & (1ULL << (rank * 8 + f)))
            break;
    }
    return attacks;
}

u64 getBishopAttacks(int sq, u64 occ)
{
    u64 attacks = 0ULL;
    int rank = sq / 8, file = sq % 8;

    for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occ & (1ULL << (r * 8 + f)))
            break;
    }
    for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occ & (1ULL << (r * 8 + f)))
            break;
    }
    for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occ & (1ULL << (r * 8 + f)))
            break;
    }
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occ & (1ULL << (r * 8 + f)))
            break;
    }
    return attacks;
}

// Masks
u64 maskRook(int sq)
{
    u64 mask = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    for (int f = file + 1; f <= 6; ++f)
        mask |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f >= 1; --f)
        mask |= 1ULL << (rank * 8 + f);
    for (int r = rank + 1; r <= 6; ++r)
        mask |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r >= 1; --r)
        mask |= 1ULL << (r * 8 + file);
    return mask;
}

u64 maskBishop(int sq)
{
    u64 mask = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
        mask |= 1ULL << (r * 8 + f);
    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
        mask |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
        mask |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
        mask |= 1ULL << (r * 8 + f);
    return mask;
}


void initMasks()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        RookMasks[sq] = maskRook(sq);
        BishopMasks[sq] = maskBishop(sq);
        rookBits[sq] = popcount(RookMasks[sq]);
        bishopBits[sq] = popcount(BishopMasks[sq]);
    }
}


void initRookAttacks()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        int bits = rookBits[sq];
        int occupancyCount = 1 << bits;
        RookAttackTable[sq].resize(occupancyCount);

        for (int index = 0; index < occupancyCount; ++index)
        {
            u64 occ = setOccupancy(index, bits, RookMasks[sq]);
            // The index must match the lookup formula used later
            int magicIndex = (occ * RookMagics[sq]) >> (64 - rookBits[sq]);
            RookAttackTable[sq][magicIndex] = getRookAttacks(sq, occ);
        }
    }
}

void initBishopAttacks()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        int bits = bishopBits[sq];
        int occupancyCount = 1 << bits;
        BishopAttackTable[sq].resize(occupancyCount);

        for (int index = 0; index < occupancyCount; ++index)
        {
            u64 occ = setOccupancy(index, bits, BishopMasks[sq]);
            // The index must match the lookup formula used later
            int magicIndex = (occ * BishopMagics[sq]) >> (64 - bishopBits[sq]);
            BishopAttackTable[sq][magicIndex] = getBishopAttacks(sq, occ);
        }
    }
}

void initKnightAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        u64 bit = 1ULL << sq;
        u64 attacks = 0;

        attacks |= (bit & ~FILE_H)  << 17;
        attacks |= (bit & ~FILE_A)  << 15;
        attacks |= (bit & ~(FILE_G | FILE_H)) << 10;
        attacks |= (bit & ~(FILE_A | FILE_B)) << 6;

        attacks |= (bit & ~FILE_H)  >> 15;
        attacks |= (bit & ~FILE_A)  >> 17;
        attacks |= (bit & ~(FILE_G | FILE_H)) >> 6;
        attacks |= (bit & ~(FILE_A | FILE_B)) >> 10;

        knightAttacks[sq] = attacks;
    }
}


void initKingAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        u64 bit = 1ULL << sq;
        u64 attacks = 0;

        attacks |= bit << 8;
        attacks |= bit >> 8;

        attacks |= (bit & ~FILE_H) << 1;
        attacks |= (bit & ~FILE_A) >> 1;

        attacks |= (bit & ~FILE_H) << 9;
        attacks |= (bit & ~FILE_A) << 7;
        attacks |= (bit & ~FILE_H) >> 7;
        attacks |= (bit & ~FILE_A) >> 9;

        kingAttacks[sq] = attacks;
    }
}


void initAttacks() {
    initRookAttacks();
    initBishopAttacks();
    initKnightAttacks();
    initKingAttacks();
}

inline u64 getRookAttackMagics(int sq, u64 occ)
{
    occ &= RookMasks[sq];
    int index = (occ * RookMagics[sq]) >> (64 - rookBits[sq]); // safe index
    return RookAttackTable[sq][index];
}

inline u64 getBishopAttackMagics(int sq, u64 occ)
{
    occ &= BishopMasks[sq];
    int index = (occ * BishopMagics[sq]) >> (64 - bishopBits[sq]); // safe index
    return BishopAttackTable[sq][index];
}

inline u64 getQueenAttackMagics(int sq, u64 occ) { return getRookAttackMagics(sq, occ) | getBishopAttackMagics(sq, occ); }




int generatePawnMoves(const Board& board, Color side, Move* moves)
{
    u64 pawns = (side == WHITE) ? board.pieces[WP] : board.pieces[BP];
    u64 empty = ~board.all;

    int count = 0;

    if (side == WHITE)
    {
        // === Single pushes ===
        u64 singlePush = (pawns << 8) & empty;

        while (singlePush)
        {
            int toSq = lsb(singlePush);
            singlePush &= singlePush - 1;

            int fromSq = toSq - 8;

            if ((1ULL << toSq) & RANK_8)
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, NONE, WQ, PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, NONE, WR, PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, NONE, WB, PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, NONE, WN, PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, NONE, NONE, QUIET);
            }
        }

        // === Double pushes ===
        u64 doublePush = ((pawns & RANK_2) << 8 & empty) << 8 & empty;

        while (doublePush)
        {
            int toSq = lsb(doublePush);
            doublePush &= doublePush - 1;

            int fromSq = toSq - 16;

            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, NONE, NONE, QUIET);
        }

        // === Captures left ===
        u64 leftCaps = (pawns << 7) & ~FILE_H & board.all_black;


        while (leftCaps)
        {
            int toSq = lsb(leftCaps);
            leftCaps &= leftCaps - 1;

            int fromSq = toSq - 7;

            if ((1ULL << toSq) & RANK_8)
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), NONE, CAPTURE);
            }
        }

        // === Captures right ===
        u64 rightCaps = (pawns << 9) & ~FILE_A & board.all_black;
        

        while (rightCaps)
        {
            int toSq = lsb(rightCaps);
            rightCaps &= rightCaps - 1;

            int fromSq = toSq - 9;

            if ((1ULL << toSq) & RANK_8)
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), NONE, CAPTURE);
            }
        }


        // En passant

        if (board.enPassantSquare != -1)
            {
                u64 epBB = 1ULL << board.enPassantSquare;

                // left EP (<<7)
                u64 epLeft = (pawns << 7) & ~FILE_H & epBB;

                while (epLeft)
                {
                    int toSq = lsb(epLeft);
                    epLeft &= epLeft - 1;

                    int fromSq = toSq - 7;

                    moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, BP, NONE, CAPTURE | ENPASSANT);
                }

                // right EP (<<9)
                u64 epRight = (pawns << 9) & ~FILE_A & epBB;

                while (epRight)
                {
                    int toSq = lsb(epRight);
                    epRight &= epRight - 1;

                    int fromSq = toSq - 9;

                    moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, BP, NONE, CAPTURE | ENPASSANT);
                }
            }
    }
    else
    {
        // === Single pushes ===
        u64 singlePush = (pawns >> 8) & empty;

        while (singlePush)
        {
            int toSq = lsb(singlePush);
            singlePush &= singlePush - 1;

            int fromSq = toSq + 8;

            if ((1ULL << toSq) & RANK_1)
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, NONE, BQ, PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, NONE, BR, PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, NONE, BB, PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, NONE, BN, PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, NONE, NONE, QUIET);
            }
        }

        // === Double pushes ===
        u64 doublePush = ((pawns & RANK_7) >> 8 & empty) >> 8 & empty;

        while (doublePush)
        {
            int toSq = lsb(doublePush);
            doublePush &= doublePush - 1;

            int fromSq = toSq + 16;

            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, NONE, NONE, QUIET);
        }

        // === Captures left ===
        u64 leftCaps = (pawns >> 9) & ~FILE_H & board.all_white;

        while (leftCaps)
        {
            int toSq = lsb(leftCaps);
            leftCaps &= leftCaps - 1;

            int fromSq = toSq + 9;

            if ((1ULL << toSq) & RANK_1)
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), NONE, CAPTURE);
            }
        }

        // === Captures right ===
        u64 rightCaps = (pawns >> 7) & ~FILE_A & board.all_white;

        while (rightCaps)
        {
            int toSq = lsb(rightCaps);
            rightCaps &= rightCaps - 1;

            int fromSq = toSq + 7;

            if ((1ULL << toSq) & RANK_1)
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), NONE, CAPTURE);
            }
        }

        // En passant

        if (board.enPassantSquare != -1)
        {
            u64 epBB = 1ULL << board.enPassantSquare;

            // left EP (>>9)
            u64 epLeft = (pawns >> 9) & ~FILE_H & epBB;

            while (epLeft)
            {
                int toSq = lsb(epLeft);
                epLeft &= epLeft - 1;

                int fromSq = toSq + 9;

                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, WP, NONE, CAPTURE | ENPASSANT);
            }

            // right EP (>>7)
            u64 epRight = (pawns >> 7) & ~FILE_A & epBB;

            while (epRight)
            {
                int toSq = lsb(epRight);
                epRight &= epRight - 1;

                int fromSq = toSq + 7;

                moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, WP, NONE, CAPTURE | ENPASSANT);
            }
        }
    }

    if (count >= 256) {
        std::cout << "OVERFLOW inside generator\n";
        exit(1);
    }
    return count;
}

int generateKnightMoves(const Board& board, Color Side, Move* moves) {

    u64 knights = (Side == WHITE) ? board.pieces[WN] : board.pieces[BN];
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;
    u64 oppPieces = (Side == WHITE) ? board.all_black : board.all_white;

    int count = 0;

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;


        u64 attacks = knightAttacks[fromSq] & ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;
            if ((1ULL << toSq) & oppPieces) {
                captured = getPieceType(board, toSq);
            } else {
                captured = NONE;
            }
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (Side == WHITE) ? WN : BN, captured, NONE, (captured == NONE) ? QUIET : CAPTURE);
        }
    }
    if (count >= 256) {
        std::cout << "OVERFLOW inside generator\n";
        exit(1);
    }

    return count;
};

int generateRookMoves(const Board& board, Color Side, Move* moves) {
    u64 rooks = (Side == WHITE) ? board.pieces[WR] : board.pieces[BR];
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;
    u64 oppPieces = (Side == WHITE) ? board.all_black : board.all_white;

    int count = 0;

    while (rooks)
    {
        
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 attacks = getRookAttackMagics(fromSq, board.all);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;
            if ((1ULL << toSq) & oppPieces) {
                captured = getPieceType(board, toSq);
            } else {
                captured = NONE;
            }
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (Side == WHITE) ? WR : BR, captured, NONE, (captured == NONE) ? QUIET : CAPTURE);
        }
    }
    if (count >= 256) {
        std::cout << "OVERFLOW inside generator\n";
        exit(1);
    }

    return count;
};

int generateBishopMoves(const Board &board, Color Side, Move* moves)
{
    u64 bishops = (Side == WHITE) ? board.pieces[WB] : board.pieces[BB];
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;
    u64 oppPieces = (Side == WHITE) ? board.all_black : board.all_white;

    int count = 0;

    while (bishops)
    {
        
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 attacks = getBishopAttackMagics(fromSq, board.all);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;
            if ((1ULL << toSq) & oppPieces) {
                captured = getPieceType(board, toSq);
            } else {
                captured = NONE;
            }
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (Side == WHITE) ? WB : BB, captured, NONE, (captured == NONE) ? QUIET : CAPTURE);
        }
    }
    if (count >= 256) {
        std::cout << "OVERFLOW inside generator\n";
        exit(1);
    }

    return count;
}

int generateQueenMoves(const Board& board, Color Side, Move* moves) {
    u64 queens = (Side == WHITE) ? board.pieces[WQ] : board.pieces[BQ];
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;
    u64 oppPieces = (Side == WHITE) ? board.all_black : board.all_white;

    int count = 0;

    while (queens)
    {
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 attacks = getQueenAttackMagics(fromSq, board.all);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;

            if ((1ULL << toSq) & oppPieces) {
                captured = getPieceType(board, toSq);
            } else {
                captured = NONE;
            }
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (Side == WHITE) ? WQ : BQ, captured, NONE, (captured == NONE) ? QUIET : CAPTURE);

            
        }
    }
    if (count >= 256) {
        std::cout << "OVERFLOW inside generator\n";
        exit(1);
    }

    return count;
}

int generateKingMoves(const Board& board, Color Side, Move* moves) {
    u64 king = (Side == WHITE) ? board.pieces[WK] : board.pieces[BK];
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;
    u64 oppPieces = (Side == WHITE) ? board.all_black : board.all_white;

    int count = 0;

    if (king)
    {

        int fromSq = lsb(king);
        u64 attacks = kingAttacks[fromSq] & ~ownPieces;


        // Normal king moves
        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;

            if ((1ULL << toSq) & oppPieces) {
                captured = getPieceType(board, toSq);
            } else {
                captured = NONE;
            }
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (Side == WHITE) ? WK : BK, captured, NONE, (captured == NONE) ? QUIET : CAPTURE);

            
        }
        
        // White King-side (O-O)
        if (Side == WHITE && fromSq == 4 && (board.castlingRights & CWK)) {
            if ((board.pieces[WR] & (1ULL << 7)) &&         // rook exists on h1
                !(board.all & ((1ULL << 5) | (1ULL << 6))) &&     // f1 and g1 empty
                !isSquareAttacked(board, WHITE, 4) &&       // e1 not attacked
                !isSquareAttacked(board, WHITE, 5) &&       // f1 not attacked
                !isSquareAttacked(board, WHITE, 6))         // g1 not attacked
            {
                moves[count++] = mkMove(4, 6, 7, 5, WK, NONE, NONE, CASTLING);
            }
        }

        // White Queen-side (O-O-O)
        if (Side == WHITE && fromSq == 4 && (board.castlingRights & CWQ)) {
            if ((board.pieces[WR] & (1ULL << 0)) &&         // rook exists on a1
                !(board.all & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) && // b1,c1,d1 empty
                !isSquareAttacked(board, WHITE, 4) &&
                !isSquareAttacked(board, WHITE, 3) &&
                !isSquareAttacked(board, WHITE, 2))
            {
                moves[count++] = mkMove(4, 2, 0, 3, WK, NONE, NONE, CASTLING);
            }
        }

        // Black King-side (O-O)
        if (Side == BLACK && fromSq == 60 && (board.castlingRights & CBK)) {
            if ((board.pieces[BR] & (1ULL << 63)) &&
                !(board.all & ((1ULL << 61) | (1ULL << 62))) &&
                !isSquareAttacked(board, BLACK, 60) &&
                !isSquareAttacked(board, BLACK, 61) &&
                !isSquareAttacked(board, BLACK, 62))
            {
                moves[count++] = mkMove(60, 62, 63, 61, BK, NONE, NONE, CASTLING);
            }
        }

        // Black Queen-side (O-O-O)
        if (Side == BLACK && fromSq == 60 && (board.castlingRights & CBQ)) {
            if ((board.pieces[BR] & (1ULL << 56)) &&
                !(board.all & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
                !isSquareAttacked(board, BLACK, 60) &&
                !isSquareAttacked(board, BLACK, 59) &&
                !isSquareAttacked(board, BLACK, 58))
            {
                moves[count++] = mkMove(60, 58, 56, 59, BK, NONE, NONE, CASTLING);
            }
        }
    }

    if (count >= 256) {
        std::cout << "OVERFLOW inside generator\n";
        exit(1);
    }
    return count;
    
};

int generatePseudoLegalMoves(const Board& board, Color Side, Move* moves) {
    
    int count = 0;
    
    count += generatePawnMoves(board, Side, moves + count);
    count += generateKnightMoves(board, Side, moves + count);
    count += generateKingMoves(board, Side, moves + count);
    count += generateRookMoves(board, Side, moves + count);
    count += generateBishopMoves(board, Side, moves + count);
    count += generateQueenMoves(board, Side, moves + count);

    if (count > 256) {
        std::cout << "Move overflow: " << count << std::endl;
        exit(1);
    }

    return count;

};


