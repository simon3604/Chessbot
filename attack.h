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

bool isMoveLegal(Board& board, Move m, Color side);

int generateLegalMoves( Board& board, Color side, Move* moves);
bool isSquareAttacked(const Board& board, Color Side, int attackedSquare);

bool isKingInCheck(Color Side, const Board &board);

int generateCaptures(Board& board, Color side, Move* moves);

int generateQuietMoves(Board& board, Move* moves);
