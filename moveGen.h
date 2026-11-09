#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include "constants.h"

using u64 = uint64_t;
using u32 = uint32_t;


int lsb(u64 bb);
int msb(u64 bb);

int popcount(u64 x);

inline int fromSquare(const u32 move) { return move & 0x3F; }
inline int toSquare(const u32 move) { return (move >> 6) & 0x3F; }
inline Piece promotionPiece(const u32 move) { 
    return static_cast<Piece>((move >> 12) & 0xF); 
}

inline int moveFlags(const u32 move) { return (move >> 16) & 0xF; }


u32 mkMove(int from, int to, Piece promotion, uint8_t flag);
u64 setOccupancy(int index, int bits, u64 mask);




// Masks
u64 maskRook(int sq);

u64 maskBishop(int sq);


void initMasks();


void initRookAttacks();

void initBishopAttacks();
void initKnightAttacks();

void initKingAttacks();

void initAttacks();


u64 getRookAttackMagics(int sq, u64 occ);

u64 getBishopAttackMagics(int sq, u64 occ);

u64 getQueenAttackMagics(int sq, u64 occ);





int generatePawnMoves(const Board& board, Color Side, u64 occ, u32* moves);

int generateKnightMoves(const Board& board, Color Side, u64 occ, u32* moves);

int generateRookMoves(const Board& board, Color Side, u64 occ, u32* moves);

int generateBishopMoves(Board &board, Color side, u64 occ, u32* moves);

int generateQueenMoves(const Board& board, Color Side, u64 occ, u32* moves);

int generateKingMoves(const Board& board, Color Side, u64 occ, u32* moves);

int generatePseudoLegalMoves(const Board& board, Color Side, u32* moves);

