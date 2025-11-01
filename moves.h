#pragma once
#include <cstdint>
#include <vector>

#include "constants.h"

using u64 = uint64_t;

bool canCastleKingside_white; 
bool canCastleQueenside_white;

bool canCastleKingside_black; 
bool canCastleQueenside_black;

std::vector<std::vector<u64>> RookAttackTable(64);
std::vector<std::vector<u64>> BishopAttackTable(64);


u64 RookMasks[64];
u64 BishopMasks[64];

int rookBits[64];
int bishopBits[64];


// LSB and MSB helpers
int lsb(u64 bb);
int msb(u64 bb);

int popcount(u64 x);


Move mkMove(int from, int to, int from2, int to2, u64 capturedPiece, Piece promotion = NONE);

u64 setOccupancy(int index, int bits, u64 mask); 

// Rook attacks on the fly
u64 getRookAttacks(int sq, u64 occ);


// Queen attacks on the fly (optional)
u64 getQueenAttacks(int sq, u64 occ);


// Piece attacks
u64 getKingAttacks(u64 bit);


u64 getKnightAttacks(u64 bit);


u64 getPawnPushes(u64 bit, Color side, const Board &board);


u64 getPawnCaptures(u64 bit, Color side, const Board &board);


// Masks
u64 maskRook(int sq);


u64 maskBishop(int sq);


void initMasks();


// Rook/Bishop attacks on the fly
u64 getRookAttackMagics(int sq, u64 occ);


u64 getBishopAttackMagics(int sq, u64 occ);


void initRookAttacks();


void initBishopAttacks();


u64 getQueenAttackMagics(int sq, u64 occ);

// Check for king in check
bool isKingInCheck(Color side, const Board &board);


std::vector<Move> generateKnightMoves(Board &board, Color side, std::vector<Move> &moves);


std::vector<Move> generateKingMoves(Board &board, Color side, std::vector<Move> &moves); 

std::vector<Move> generatePawnMoves(Board &board, Color side, std::vector<Move> moves);

std::vector<Move> generateRookMoves(Board &board, Color side, std::vector<Move> &moves);


std::vector<Move> generateBishopMoves(Board &board, Color side, std::vector<Move> &moves);


std::vector<Move> generateQueenMoves(Board &board, Color side, std::vector<Move> &moves);
