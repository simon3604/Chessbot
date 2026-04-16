#pragma once

#include <cstdint>
#include <vector>
#include <chrono>

#include "moveGen.h"
#include "evaluation.h"

using u64 = uint64_t;

int quiescence(Board& board, int alpha, int beta, Color side, int ply);

int alphaBeta(Board& board, int depth, int alpha, int beta, Color side, int ply, int time, std::chrono::steady_clock::time_point start_time, bool nullAllowed);


Move findBestMove(Board& board, int depth, int time, std::chrono::steady_clock::time_point start_time);

Move search(Board& board, GoParams go);