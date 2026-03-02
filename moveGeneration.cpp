#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>
#include "constants.h"
#include "moveGeneration.h"
#include "misc.h"

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


Move mkMove(int from, int to, int from2, int to2, u64 capturedPiece, Piece promotion)
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

u64 getKnightAttacks(u64 bit) {

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


bool isKingInCheck(Color side, const Board &board)
{
    u64 occ = board.pawns_white | board.knights_white | board.bishops_white |
              board.rooks_white | board.queens_white | board.king_white |
              board.pawns_black | board.knights_black | board.bishops_black |
              board.rooks_black | board.queens_black | board.king_black;

    u64 kingBB = (side == WHITE) ? board.king_white : board.king_black;
    if (!kingBB) {
        std::cerr << "isKingInCheck: No King " << std::endl;
        return false;
    }

    int kingSq = lsb(kingBB);

    u64 pawnAttacks = (side == WHITE) ? 
        ((board.pawns_black >> 7) & ~FILE_H) |
        ((board.pawns_black >> 9) & ~FILE_A) 
        : 
        ((board.pawns_white << 7) & ~FILE_A) |
        ((board.pawns_white << 9) & ~FILE_H);
    
    if (pawnAttacks & kingBB) {
        logToFile("isKingInCheck: Pawn gives check!");
        return true;
    }

    if (getKnightAttacks(1ULL << kingSq) & ((side == WHITE) ? board.knights_black : board.knights_white)) {
        logToFile("isKingInCheck: Knight gives check!");
        return true;
    }

    if (getBishopAttackMagics(kingSq, occ) & ((side == WHITE) ? board.bishops_black | board.queens_black : board.bishops_white | board.queens_white)) {
        logToFile("isKingInCheck: Bishop/Queen gives check!");
        return true;
    }

    if (getRookAttackMagics(kingSq, occ) & ((side == WHITE) ? board.rooks_black | board.queens_black : board.rooks_white | board.queens_white)) {
        logToFile("isKingInCheck: Rook/Queen gives check!");
        return true;
    }

    if (getKingAttacks(1ULL << kingSq) & ((side == WHITE) ? board.king_black : board.king_white)) {
        logToFile("isKingInCheck: King gives check!");
        return true;
    }


    return false;
}


std::vector<Move> generateKnightMoves(Board &board, Color side, std::vector<Move> &moves) {

    
    
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

std::vector<Move> generatePawnMoves(Board &board, Color side, std::vector<Move> &moves) {

    u64 pawns = (side == WHITE) ? board.pawns_white : board.pawns_black;
    u64 empty = ~(board.all_white | board.all_black);

    while (pawns) {
        int fromSq = lsb(pawns);
        pawns &= pawns - 1;
        
        u64 pushTargets = getPawnPushes(1ULL << fromSq, side, board);
        u64 capTargets = getPawnCaptures(1ULL << fromSq, side, board);
        int ep = board.enPassantSquare;
        
        // === Handle pawn pushes ===
        while (pushTargets)
        {
            
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
                std::cerr << "promotion push: " << fromSq << " → " << toSq << "\n";
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
                std::cerr << "promotion capture: " << fromSq << " → " << toSq << "\n";
                
                
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
                // std::cout << "move: " << fromSq << " → " << toSq << "\n";
            }
        }

        //En Passant
        if (ep != -1) {
        u64 epBit = 1ULL << ep;

        if (side == WHITE) {
            if (fromSq % 8 != 0 && epBit == (1ULL << (fromSq + 7)))  // capture left
                moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep - 8)));
            if (fromSq % 8 != 7 && epBit == (1ULL << (fromSq + 9)))  // capture right
                moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep - 8)));
        } else {
            if (fromSq % 8 != 0 && epBit == (1ULL << (fromSq - 9)))
                moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep + 8)));
            if (fromSq % 8 != 7 && epBit == (1ULL << (fromSq - 7)))
                moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep + 8)));
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


u64* getTypeBB(int pos, Board& board) {
    u64 mask = 1ULL << pos;
    if (mask & board.pawns_white)  return &board.pawns_white;
    if (mask & board.rooks_white)  return &board.rooks_white;
    if (mask & board.knights_white) return &board.knights_white;
    if (mask & board.bishops_white) return &board.bishops_white;
    if (mask & board.queens_white)  return &board.queens_white;
    if (mask & board.king_white)   return &board.king_white;

    if (mask & board.pawns_black)  return &board.pawns_black;
    if (mask & board.rooks_black)  return &board.rooks_black;
    if (mask & board.knights_black) return &board.knights_black;
    if (mask & board.bishops_black) return &board.bishops_black;
    if (mask & board.queens_black)  return &board.queens_black;
    if (mask & board.king_black)   return &board.king_black;

    return nullptr;
}
void check_board_integrity(const Board &board, int square, const char* tag = "") {
    u64 all = 0ULL;
    std::string prefix = tag ? std::string(tag) + ": " : "";
    auto check_and_report = [&](u64 bb, const char* name) {
        if (bb & all) {
            std::cerr << square << prefix << "OVERLAP detected for " << name << "\n";
        }
        all |= bb;
    };

    check_and_report(board.pawns_white,   "pawns_white");
    check_and_report(board.knights_white, "knights_white");
    check_and_report(board.bishops_white, "bishops_white");
    check_and_report(board.rooks_white,   "rooks_white");
    check_and_report(board.queens_white,  "queens_white");
    check_and_report(board.king_white,    "king_white");

    check_and_report(board.pawns_black,   "pawns_black");
    check_and_report(board.knights_black, "knights_black");
    check_and_report(board.bishops_black, "bishops_black");
    check_and_report(board.rooks_black,   "rooks_black");
    check_and_report(board.queens_black,  "queens_black");
    check_and_report(board.king_black,    "king_black");

    // check all_white/all_black are consistent:
    u64 recomputed_white = board.pawns_white | board.knights_white | board.bishops_white |
                           board.rooks_white  | board.queens_white  | board.king_white;
    u64 recomputed_black = board.pawns_black | board.knights_black | board.bishops_black |
                           board.rooks_black  | board.queens_black  | board.king_black;

    if (recomputed_white != board.all_white)
        std::cerr << prefix << "all_white mismatch: recomputed=" << std::hex << recomputed_white
                  << " stored=" << board.all_white << std::dec << "\n";
    if (recomputed_black != board.all_black)
        std::cerr << prefix << "all_black mismatch: recomputed=" << std::hex << recomputed_black
                  << " stored=" << board.all_black << std::dec << "\n";

    // check bit 0 specifically
    // bool bit0set = ((board.queens_white | board.rooks_white | board.pawns_white |
    //                  board.knights_white | board.bishops_white) & 1ULL) != 0;
    // if (bit0set) {
    //     std::cerr << prefix << "bit 0 is set in white piece bitboards!\n";
    //     print_bitboard(board.queens_white);
    //     print_bitboard(board.rooks_white);
    //     print_bitboard(board.pawns_white);
    // }
}




u64 movePiece(u64 &pieceBB, int fromSq, int toSq, Color side, Board &board) {
    u64 captured = 0ULL;

    if (side == WHITE) {
        if ((1ULL << toSq) & board.all_black) {
            u64* captureBB = getTypeBB(toSq, board);
            if (captureBB) {
                captured = *captureBB & (1ULL << toSq);
                *captureBB &= ~(1ULL << toSq);
            }
        }
    } else {
        if ((1ULL << toSq) & board.all_white) {
            u64* captureBB = getTypeBB(toSq, board);
            if (captureBB) {
                captured = *captureBB & (1ULL << toSq);
                *captureBB &= ~(1ULL << toSq);
            }
        }
    }

    // move the piece
    pieceBB &= ~(1ULL << fromSq);
    pieceBB |= (1ULL << toSq);

    // update all_white/all_black
    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white | board.queens_white | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black | board.queens_black | board.king_black;

    return captured;
}



// Undo a move
void undoMove(Move m, Board& board, Color side, Undo undo) {
    // If the move was a promotion: remove promoted piece and restore pawn
    if (m.promotion != NONE) {
        if (undo.movedPiece) {
            // remove the promoted piece from its destination square
            *undo.movedPiece &= ~(1ULL << m.to);
        }
        // restore pawn at the original square
        u64* pawnBB = (side == WHITE) ? &board.pawns_white : &board.pawns_black;
        *pawnBB |= (1ULL << m.from);
    } else {
        // Normal move undo: move the bit back from 'to' -> 'from'
        if (undo.movedPiece) {
            *undo.movedPiece &= ~(1ULL << m.to);
            *undo.movedPiece |=  (1ULL << m.from);
        }
    }

    // Restore captured piece (if any)
    if (undo.captured && undo.capturedPiece) {
        *undo.capturedPiece |= undo.captured;
    }

    // Undo rook move from castling
    if (undo.movedRook) {
        *undo.movedRook &= ~(1ULL << m.to2);
        *undo.movedRook |=  (1ULL << m.from2);
    }

    // Recompute occupancy masks (always do this after changes)
    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white  | board.queens_white  | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black  | board.queens_black  | board.king_black;
    //std::cout << "undoMove";             
    check_board_integrity(board, m.to); 
             
}


// Make a move on the board and return undo info
Undo makeMove(Move m, Board& board, Color side) {
    Undo undo;
    undo.from = m.from;
    undo.to = m.to;
    undo.from2 = m.from2;
    undo.to2 = m.to2;
    undo.movedPiece = getTypeBB(m.from, board);
    undo.captured = 0ULL;
    undo.capturedPiece = nullptr;
    undo.movedRook = nullptr; 

    if (!undo.movedPiece) return undo; // safety check

    // handle capture
    u64 targetMask = 1ULL << m.to;

    if (side == WHITE && (targetMask & board.all_black)) {
        undo.capturedPiece = getTypeBB(m.to, board);
        if (undo.capturedPiece) {
            undo.captured = targetMask;
            *undo.capturedPiece &= ~targetMask;
        }
    } else if (side == BLACK && (targetMask & board.all_white)) {
        undo.capturedPiece = getTypeBB(m.to, board);
        if (undo.capturedPiece) {
            undo.captured = targetMask;
            *undo.capturedPiece &= ~targetMask;
        }
    }


    if ((m.promotion != NONE) && !((1ULL << m.from) & ((side == WHITE) ? board.pawns_white : board.pawns_black))) {
    std::cerr << "⚠️ ERROR: Promotion set on a non-pawn move! from=" << m.from
              << " to=" << m.to << "\n";
    }
    

    u64 maskFrom = 1ULL << m.from;
    bool isPawn = (side == WHITE)
        ? (maskFrom & board.pawns_white)
        : (maskFrom & board.pawns_black);

    // std::cerr << "makeMove: from " << m.from
    //         << " to " << m.to
    //         << " promo=" << m.promotion
    //         << " isPawn=" << isPawn
    //         << "\n";


    // === Handle promotion ===
    if (m.promotion != NONE) {
        // Remove pawn from its bitboard
        *undo.movedPiece &= ~(1ULL << m.from);

        // Add promoted piece to the right bitboard
        u64* promoBB = nullptr;
        switch (m.promotion) {
            case QUEEN:
                promoBB = (side == WHITE) ? &board.queens_white : &board.queens_black;
                break;
            case ROOK:
                promoBB = (side == WHITE) ? &board.rooks_white : &board.rooks_black;
                break;
            case BISHOP:
                promoBB = (side == WHITE) ? &board.bishops_white : &board.bishops_black;
                break;
            case KNIGHT:
                promoBB = (side == WHITE) ? &board.knights_white : &board.knights_black;
                break;
            default:
                promoBB = undo.movedPiece; // just in case
                break;
        }

        *promoBB |= (1ULL << m.to);
        undo.movedPiece = promoBB; // track promoted piece for undo
    }
    else {
        // === Normal move ===
        *undo.movedPiece &= ~(1ULL << m.from);
        *undo.movedPiece |=  (1ULL << m.to);
    }

    // === Handle castling rook ===
    if (m.from2 != -1 && m.to2 != -1) {
        undo.movedRook = getTypeBB(m.from2, board);
        if (undo.movedRook) {
            *undo.movedRook &= ~(1ULL << m.from2);
            *undo.movedRook |=  (1ULL << m.to2);
        }
    }

    // if (isKingInCheck(side, board)) {
    //     undoMove(m, board, side, undo);
    //     undo.to = -1;
    //     return undo;
    // }

    // === En Passant === 
    if ((*undo.movedPiece == (board.pawns_black || board.pawns_white)) && abs(undo.to - undo.from) == 16) {
        board.enPassantSquare = (side == WHITE) 
        ? undo.from + 8 : undo.from - 8; 
    } else {
        board.enPassantSquare = -1;
    }


    // === Update bitboards ===
    board.all_white =
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black =
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    //std::cout << "MakeMove";
    check_board_integrity(board, m.to);
    return undo;
}


std::vector<Move> generateMoves(Board& board, Color side, std::vector<Move>& moves) {
   
    
    
    
    moves.clear();
    std::cerr << "Generated moves:";
    generateKnightMoves(board, side, moves);
    std::cerr << moves.size() << "Knight" << std::endl;
    generateKingMoves(board, side, moves);
    std::cerr << moves.size() << "King" << std::endl;
    generatePawnMoves(board, side, moves);
    std::cerr << moves.size() << "Pawn" << std::endl;
    generateRookMoves(board, side, moves);
    std::cerr << moves.size() << "Rook" << std::endl;
    generateBishopMoves(board, side, moves);
    std::cerr << moves.size() << "Bishop" << std::endl;
    generateQueenMoves(board, side, moves);
    std::cerr << moves.size() << "Queen" << std::endl;
    //print_bitboard(board.pawns_white);
    std::vector<Move> legalMoves;
    for (auto &m : moves) {
        Undo u = makeMove(m, board, side);
        if (!isKingInCheck(side, board))
        {
            legalMoves.push_back(m);
        }
        undoMove(m, board, side, u);
    }
    std::cerr << "generateMoves: legalMoves = " << legalMoves.size() << std::endl;
    

    // --- Castling ---
    u64 allPieces = board.all_white | board.all_black;
    if (side == WHITE)
    {
        // White kingside (E1 -> G1, rook H1 -> F1)
        if (canCastleKingside_white &&
            !(allPieces & ((1ULL << 5) | (1ULL << 6))) && // F1, G1 empty
            !isKingInCheck(WHITE, board))
        {

            moves.push_back(mkMove(4, 6, 7, 5, 0ULL, NONE)); // from2=7 (rook), to2=5 (rook move)
        }

        // White queenside (E1 -> C1, rook A1 -> D1)
        if (canCastleQueenside_white &&
            !(allPieces & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) && // B1, C1, D1 empty
            !isKingInCheck(WHITE, board))
        {

            Board temp = board;
            movePiece(temp.king_white, 4, 3, WHITE, temp);
            if (!isKingInCheck(WHITE, temp))
            {
                movePiece(temp.king_white, 3, 2, WHITE, temp);
                if (!isKingInCheck(WHITE, temp))
                    moves.push_back(mkMove(4, 2, 0, 3, 0)); // from2=0 (rook), to2=3 (rook move)
                                                            // std::cout << "move: " << fromSq << " → " << "K" << "\n";
            }
        }
    }
    else
    {
        // Black kingside (E8 -> G8, rook H8 -> F8)
        if (canCastleKingside_black &&
            !(allPieces & ((1ULL << 61) | (1ULL << 62))) && // F8, G8 empty
            !isKingInCheck(BLACK, board))
        {

            Board temp = board;
            movePiece(temp.king_black, 60, 61, BLACK, temp);
            if (!isKingInCheck(BLACK, temp))
            {
                movePiece(temp.king_black, 61, 62, BLACK, temp);
                if (!isKingInCheck(BLACK, temp))
                    moves.push_back(mkMove(60, 62, 63, 61, 0));
                // std::cout << "move: " << fromSq << " → "  << "K" << "\n";
            }
        }

        // Black queenside (E8 -> C8, rook A8 -> D8)
        if (canCastleQueenside_black &&
            !(allPieces & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
            !isKingInCheck(BLACK, board))
        {

            Board temp = board;
            movePiece(temp.king_black, 60, 59, BLACK, temp);
            if (!isKingInCheck(BLACK, temp))
            {
                movePiece(temp.king_black, 59, 58, BLACK, temp);
                if (!isKingInCheck(BLACK, temp))
                    moves.push_back(mkMove(60, 58, 56, 59, 0));
                // std::cout << "move: " << fromSq << " → "  << "K" << "\n";
            }
        }
    }
    moves = legalMoves;
    return moves;
}

