#pragma once

#include <cstdint>
#include <vector>

#include "moveGen.h"
#include "evaluation.h"

using u64 = uint64_t;



int alphaBeta(Board& board, int depth, int alpha, int beta, Color side, int ply);


Move findBestMove(Board& board, Color side, int depth);

Move search(Board& board, Color side, int depth);