#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include "constants.h"
#include "moveGen.h"
#include "misc.h"

using u32 = uint32_t;

bool isMoveLegal(Board& board, u32 m, std::string debugInfo);

int generateLegalMoves( Board& board, Color side, u32* moves);
bool isSquareAttacked(const Board& board, Color Side, int attackedSquare);

bool isKingInCheck(Color Side, const Board &board, std::string debugInfo);

int generateCaptures(Board& board, Color side, u32* moves, std::string debugInfo);

int generateQuietMoves(Board& board, Color side, u32* moves);
