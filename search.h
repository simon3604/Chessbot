#pragma once

#include <cstdint>
#include <vector>

#include "moves.h"

using u64 = uint64_t;




u64* getTypeBB(int pos, Board& board);

//Old makeMove function
u64 movePiece(u64 &pieceBB, int fromSq, int toSq, Color side, Board &board);

void check_board_integrity(const Board &board, int square, const char* tag = "");


// Undo a move
void undoMove(Move m, Board& board, Color side, Undo undo);


// Make a move on the board and return undo info
Undo makeMove(Move m, Board& board, Color side);