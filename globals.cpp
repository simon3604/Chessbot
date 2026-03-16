
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

bool canCastleKingside_white = true; 
bool canCastleQueenside_white = true;

bool canCastleKingside_black = true ; 
bool canCastleQueenside_black = true;

Color sideToMove;