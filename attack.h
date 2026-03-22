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

void generateLegalMoves( Board& board, Color side, std::vector<Move>& moves);
bool isSquareAttacked(const Board& board, Color Side, int attackedSquare);

bool isKingInCheck(Color Side, const Board &board);

void generateLegalCaptures(Board& board, Color side, std::vector<Move>& moves);

