
#include <iostream>
#include <array>
#include <vector>


#include "constants.h"

u64 RookMasks[64];
u64 BishopMasks[64];

int rookBits[64];
int bishopBits[64];

u64 knightAttacks[64];
u64 kingAttacks[64];

std::vector<std::vector<u64>> RookAttackTable(64);
std::vector<std::vector<u64>> BishopAttackTable(64);

bool canCastleKingside_white = false; 
bool canCastleQueenside_white = false;

bool canCastleKingside_black = false; 
bool canCastleQueenside_black = false;

Color sideToMove;

u64 nodes = 0;

bool perfTest = false;


Move killerMoves[64][2]; // [depth][2 slots]

int history[2][64][64]; // side, from, to

u64 zobrist[2][6][64]; // [color][piece][square]
u64 zobristSide;