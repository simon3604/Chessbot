#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
//#include <stdc++.h>
#include <cstdlib>
#include <ctime>
#include "constants.h"

using u64 = uint64_t;

// LSB and MSB helpers
int lsb(u64 bb) { return __builtin_ctzll(bb); }
int msb(u64 bb) { return 63 - __builtin_clzll(bb); }

int popcount(u64 x) { return __builtin_popcountll(x); }

bool canCastleKingside_white = true; 
bool canCastleQueenside_white = true;

bool canCastleKingside_black = true; 
bool canCastleQueenside_black = true;

std::vector<std::vector<u64>> RookAttackTable(64);
std::vector<std::vector<u64>> BishopAttackTable(64);

u64 RookMasks[64];
u64 BishopMasks[64];

int rookBits[64];
int bishopBits[64];

Move mkMove(int from, int to, int from2, int to2, u64 capturedPiece, Piece promotion = NONE)
{
    Move m;
    m.from = from;
    m.to = to;
    m.from2 = from2;
    m.to2 = to2;
    m.capturedPiece = capturedPiece;
    m.promotion = promotion;
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

// Rook attacks on the fly
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

// Bishop attacks on the fly
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

// Queen attacks on the fly (optional)
u64 getQueenAttacks(int sq, u64 occ)
{
    return getRookAttacks(sq, occ) | getBishopAttacks(sq, occ);
}

// Piece attacks
u64 getKingAttacks(u64 bit)
{
    u64 attacks = 0ULL;
    attacks |= (bit << 8);
    attacks |= (bit >> 8);
    attacks |= (bit << 1) & ~FILE_H;
    attacks |= (bit >> 1) & ~FILE_A;
    attacks |= (bit << 9) & ~FILE_A;
    attacks |= (bit << 7) & ~FILE_H;
    attacks |= (bit >> 7) & ~FILE_A;
    attacks |= (bit >> 9) & ~FILE_H;
    return attacks;
}

u64 getKnightAttacks(u64 bit)
{
    u64 attacks = 0ULL;
    attacks |= (bit << 17) & ~FILE_A;
    attacks |= (bit << 15) & ~FILE_H;
    attacks |= (bit << 10) & ~(FILE_A | FILE_B);
    attacks |= (bit << 6) & ~(FILE_H | FILE_G);
    attacks |= (bit >> 17) & ~FILE_H;
    attacks |= (bit >> 15) & ~FILE_A;
    attacks |= (bit >> 10) & ~(FILE_H | FILE_G);
    attacks |= (bit >> 6) & ~(FILE_A | FILE_B);
    return attacks;
}

u64 getPawnPushes(u64 bit, Color side, const Board &board)
{
    u64 occ = board.all_white | board.all_black;
    u64 moves = 0ULL;

    if (side == WHITE)
    {

        u64 oneAhead = bit << 8;
        if (!(oneAhead & occ))
        {
            moves |= oneAhead;
            if ((bit & RANK_2) && !(oneAhead << 8 & occ))
                moves |= oneAhead << 8;
        }
    }
    else
    {

        u64 oneAhead = bit >> 8;
        if (!(oneAhead & occ))
        {
            moves |= oneAhead;
            if ((bit & RANK_7) && !(oneAhead >> 8 & occ))
                moves |= oneAhead >> 8;
        }
    }
    return moves;
}

u64 getPawnCaptures(u64 bit, Color side, const Board &board)
{
    u64 moves = 0ULL;
    if (side == WHITE)
    {
        moves |= (bit << 7) & ~FILE_H & board.all_black;
        moves |= (bit << 9) & ~FILE_A & board.all_black;
    }
    else
    {
        moves |= (bit >> 7) & ~FILE_A & board.all_white;
        moves |= (bit >> 9) & ~FILE_H & board.all_white;
    }
    return moves;
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

// Rook/Bishop attacks on the fly
u64 getRookAttackMagics(int sq, u64 occ)
{
    occ &= RookMasks[sq];
    int index = (occ * RookMagics[sq]) >> (64 - rookBits[sq]); // safe index
    return RookAttackTable[sq][index];
}

u64 getBishopAttackMagics(int sq, u64 occ)
{
    occ &= BishopMasks[sq];
    int index = (occ * BishopMagics[sq]) >> (64 - bishopBits[sq]); // safe index
    return BishopAttackTable[sq][index];
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

u64 getQueenAttackMagics(int sq, u64 occ) { return getRookAttackMagics(sq, occ) | getBishopAttackMagics(sq, occ); }

// Check for king in check
bool isKingInCheck(Color side, const Board &board)
{
    u64 occ = board.pawns_white | board.knights_white | board.bishops_white |
              board.rooks_white | board.queens_white | board.king_white |
              board.pawns_black | board.knights_black | board.bishops_black |
              board.rooks_black | board.queens_black | board.king_black;

    u64 kingBB = (side == WHITE) ? board.king_white : board.king_black;
    if (!kingBB)
        return false;
    int kingSq = lsb(kingBB);

    if (side == WHITE)
    {
        u64 pawnAttacks = ((board.pawns_black >> 7) & ~FILE_H) |
                          ((board.pawns_black >> 9) & ~FILE_A);
        if (pawnAttacks & kingBB)
            return true;
        if (getKnightAttacks(1ULL << kingSq) & board.knights_black)
            return true;
        if (getBishopAttackMagics(kingSq, occ) & (board.bishops_black | board.queens_black))
            return true;
        if (getRookAttackMagics(kingSq, occ) & (board.rooks_black | board.queens_black))
            return true;
        if (getKingAttacks(1ULL << kingSq) & board.king_black)
            return true;
    }
    else
    {
        u64 pawnAttacks = ((board.pawns_white << 7) & ~FILE_A) |
                          ((board.pawns_white << 9) & ~FILE_H);
        if (pawnAttacks & kingBB)
            return true;
        if (getKnightAttacks(1ULL << kingSq) & board.knights_white)
            return true;
        if (getBishopAttackMagics(kingSq, occ) & (board.bishops_white | board.queens_white))
            return true;
        if (getRookAttackMagics(kingSq, occ) & (board.rooks_white | board.queens_white))
            return true;
        if (getKingAttacks(1ULL << kingSq) & board.king_white)
            return true;
    }

    return false;
}

std::vector<Move> generateKnightMoves(Board &board, Color side, std::vector<Move> &moves)
{

    u64 knights = (side == WHITE) ? board.knights_white : board.knights_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;

        u64 attacks = getKnightAttacks(1ULL << fromSq);
        attacks &= ~ownPieces; // can't capture own pieces

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
            // std::cout << "move: " << fromSq << " → " << toSq << "N" "\n";
        }
    }

    

    return moves;
}

std::vector<Move> generateKingMoves(Board &board, Color side, std::vector<Move> &moves) {
    u64 king = (side == WHITE) ? board.king_white : board.king_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    if (king)
    {

        int fromSq = lsb(king);
        u64 attacks = getKingAttacks(1ULL << fromSq);
        attacks &= ~ownPieces;

        // Normal king moves
        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
            // std::cout << "move: " << fromSq << " → " << toSq << "Kk" << "\n";
        }

    }
    return moves;
}

std::vector<Move> generatePawnMoves(Board &board, Color side, std::vector<Move> moves) {

    u64 pawns = (side == WHITE) ? board.pawns_white : board.pawns_black;
    u64 empty = ~(board.all_white | board.all_black);

    while (pawns) {
        int fromSq = lsb(pawns);
        pawns &= pawns - 1;

        u64 pushTargets = getPawnPushes(1ULL << fromSq, side, board);
        u64 capTargets = getPawnCaptures(1ULL << fromSq, side, board);
        // === Handle pawn pushes ===
        while (pushTargets)
        {
            std::cout << "hu\n";
            int toSq = lsb(pushTargets);
            pushTargets &= pushTargets - 1;

            // If the target square is the last rank -> promotion
            if ((side == WHITE && (1ULL << toSq) & RANK_8) ||
                (side == BLACK && (1ULL << toSq) & RANK_1))
            {

                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, KNIGHT));
                std::cout << "promotion push: " << fromSq << " → " << toSq << "\n";
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
                // std::cout << "move: " << fromSq << " → " << toSq << "\n";
            }
        }

        // === Handle captures ===
        while (capTargets)
        {
            int toSq = lsb(capTargets);
            capTargets &= capTargets - 1;

            if ((side == WHITE && (1ULL << toSq) & RANK_8) ||
                (side == BLACK && (1ULL << toSq) & RANK_1))
            {

                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, KNIGHT));
                std::cout << "promotion capture: " << fromSq << " → " << toSq << "\n";
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
                // std::cout << "move: " << fromSq << " → " << toSq << "\n";
            }
        }
    }

    return moves;
}

std::vector<Move> generateRookMoves(Board &board, Color side, std::vector<Move> &moves)
{
    u64 rooks = (side == WHITE) ? board.rooks_white : board.rooks_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (rooks)
    {
        u64 occ = board.all_white | board.all_black;
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 attacks = getRookAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
            // std::cout << "move: " << fromSq << " → " << "R" << "\n";
        }
    }

    return moves;
}

std::vector<Move> generateBishopMoves(Board &board, Color side, std::vector<Move> &moves)
{
    u64 bishops = (side == WHITE) ? board.bishops_white : board.bishops_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (bishops)
    {
        u64 occ = board.all_white | board.all_black;
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 attacks = getBishopAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
            // std::cout << "move: " << fromSq << " → " << "\n";
        }
    }

    return moves;
}

std::vector<Move> generateQueenMoves(Board &board, Color side, std::vector<Move> &moves)
{
    u64 queens = (side == WHITE) ? board.queens_white : board.queens_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (queens)
    {
        u64 occ = board.all_white | board.all_black;
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 attacks = getQueenAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Move m = mkMove(fromSq, toSq, -1, -1, 0ULL, NONE);
            m.promotion = NONE;
            moves.push_back(m);
            // std::cout << "move: " << fromSq << " → " << "\n";
            // std::cout << fromSq << "->" << toSq << std::endl;
        }
    }

    return moves;
}


