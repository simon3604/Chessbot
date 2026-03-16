#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>
#include "constants.h"

void print_bitboard(u64 bb);

std::string print_bitboardForLog(u64 bb);

void logToFile(const std::string message);

void checkBoard(const Board& board);

std::string printBoardAsLetters(const Board& board, bool forLog);

const u64* getTypeBB(int pos,const Board& board);

u64* getTypeBB(int pos, Board& board);
//void check_board_integrity(const Board &board, int square, const char* tag = "");




std::string numToPos(int num);

// FEN -> Board
Board fenUnloader(const std::string& fen);



std::string randomMove(std::vector<Move>& moves, Color side, Board& board);


std::string evalMove(std::vector<Move>& moves, Color side, Board& board);


int pieceValueAtSq(const Board& board, int sq);



Move parseUCIMove(const std::string& moveStr, Board& board);


void parsePositionCommand(const std::string& input, Board& board, Color side);

bool sameBoard(const Board& a, const Board& b);

Piece getPieceType(const Board& board, int sq);