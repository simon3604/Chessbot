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


int lsb(u64 bb);
int msb(u64 bb);

int popcount(u64 x);


Move mkMove(int from, int to, int from2, int to2, Piece captured, Piece promotion);
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





void generatePawnMoves(const Board& board, Color Side, std::vector<Move>& moves);

void generateKnightMoves(const Board& board, Color Side, std::vector<Move>& moves);

void generateRookMoves(const Board& board, Color Side, std::vector<Move>& moves);

void generateBishopMoves(Board &board, Color side, std::vector<Move> &moves);

void generateQueenMoves(const Board& board, Color Side, std::vector<Move>& moves);

void generateKingMoves(const Board& board, Color Side, std::vector<Move>& moves);

void generatePseudoLegalMoves(const Board& board, Color Side, std::vector<Move>& moves);

