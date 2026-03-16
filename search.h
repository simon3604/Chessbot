#pragma once

#include <cstdint>
#include <vector>

#include "moveGen.h"
#include "evaluation.h"

using u64 = uint64_t;



int alphaBeta(Board& board, int depth, int alpha, int beta, Color side, std::vector<Move> moves);


Move findBestMove(Board& board, Color side, int depth);