#pragma once

#include <vector>
#include "evaluation.h"
#include "makeMove.h"
#include "constants.h"
#include "moveGen.h"
#include "attack.h"

int kingSafety(const Board& board, Color side);
int evaluate(const Board& board, Color side); 

