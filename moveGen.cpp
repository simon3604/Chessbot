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


Move mkMove(int from, int to, int from2, int to2, Piece captured, Piece promotion)
{
    Move m;
    m.from = from;
    m.to = to;
    m.from2 = from2;
    m.to2 = to2;
    m.captured = captured;
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
            RookAttackTable[sq][magicIndex] = getRookAttackMagics(sq, occ);
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
            BishopAttackTable[sq][magicIndex] = getBishopAttackMagics(sq, occ);
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

u64 getQueenAttackMagics(int sq, u64 occ) { return getRookAttackMagics(sq, occ) | getBishopAttackMagics(sq, occ); }




void generatePawnMoves(const Board& board, Color side, u64 occ, std::vector<Move>& moves)
{
    u64 pawns = (side == WHITE) ? board.pawns_white : board.pawns_black;
    u64 empty = ~(board.all_white | board.all_black);

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
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, KNIGHT));
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
            }
        }

        // === Double pushes ===
        u64 doublePush = ((pawns & RANK_2) << 8 & empty) << 8 & empty;

        while (doublePush)
        {
            int toSq = lsb(doublePush);
            doublePush &= doublePush - 1;

            int fromSq = toSq - 16;

            moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
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
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, KNIGHT));
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
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
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, KNIGHT));
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
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
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, KNIGHT));
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
            }
        }

        // === Double pushes ===
        u64 doublePush = ((pawns & RANK_7) >> 8 & empty) >> 8 & empty;

        while (doublePush)
        {
            int toSq = lsb(doublePush);
            doublePush &= doublePush - 1;

            int fromSq = toSq + 16;

            moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
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
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, KNIGHT));
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
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
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, KNIGHT));
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
            }
        }
    }
}

void generateKnightMoves(const Board& board, Color Side, u64 occ, std::vector<Move>& moves) {

    u64 knights = (Side == WHITE) ? board.knights_white : board.knights_black;
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;


        u64 attacks = knightAttacks[fromSq] & ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
        }
    }

    

    
};

void generateRookMoves(const Board& board, Color Side, u64 occ, std::vector<Move>& moves) {
    u64 rooks = (Side == WHITE) ? board.rooks_white : board.rooks_black;
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;

    while (rooks)
    {
        
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 attacks = getRookAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
            // std::cout << "move: " << fromSq << " → " << "R" << "\n";
        }
    }

    
};

void generateBishopMoves(const Board &board, Color Side, u64 occ, std::vector<Move> &moves)
{
    u64 bishops = (Side == WHITE) ? board.bishops_white : board.bishops_black;
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;

    while (bishops)
    {
        
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 attacks = getBishopAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
            // std::cout << "move: " << fromSq << " → " << "\n";
        }
    }

    
}

void generateQueenMoves(const Board& board, Color Side, u64 occ, std::vector<Move>& moves) {
    u64 queens = (Side == WHITE) ? board.queens_white : board.queens_black;
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;

    while (queens)
    {
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 attacks = getQueenAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Move m = mkMove(fromSq, toSq, -1, -1, NONE, NONE);
            m.promotion = NONE;
            moves.push_back(m);
            // std::cout << "move: " << fromSq << " → " << "\n";
            // std::cout << fromSq << "->" << toSq << std::endl;
        }
    }

    
}

void generateKingMoves(const Board& board, Color Side, u64 occ, std::vector<Move>& moves) {
    u64 king = (Side == WHITE) ? board.king_white : board.king_black;
    u64 ownPieces = (Side == WHITE) ? board.all_white : board.all_black;

    if (king)
    {

        int fromSq = lsb(king);
        u64 attacks = kingAttacks[fromSq] & ~ownPieces;


        // Normal king moves
        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(mkMove(fromSq, toSq, -1, -1, NONE, NONE));
            // std::cout << "move: " << fromSq << " → " << toSq << "Kk" << "\n";
        }

    }
    
};

void generatePseudoLegalMoves(const Board& board, Color Side, std::vector<Move>& moves) {
    u64 occ = board.all_white | board.all_black;
    moves.clear();
    moves.reserve(256);
    
    generatePawnMoves(board, Side, occ, moves);
    generateKnightMoves(board, Side, occ, moves);
    generateKingMoves(board, Side, occ, moves);
    generateRookMoves(board, Side, occ, moves);
    generateBishopMoves(board, Side, occ, moves);
    generateQueenMoves(board, Side, occ, moves);

};


