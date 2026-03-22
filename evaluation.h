#pragma once

#include <vector>
#include "evaluation.h"
#include "makeMove.h"
#include "constants.h"
#include "moveGen.h"
#include "attack.h"

int evaluate(const Board& board, Color side); 

int scoreMove(const Board& board, Move& m, Color side, int depth);