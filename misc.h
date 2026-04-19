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
#include <random>
#include "constants.h"


void seedRNG(uint64_t seed = 5489ULL);

std::string numToPiece(int num);
void printMove(Move m);
void benchmarkSearch(Board board, int depth, Color side);

void print_bitboard(u64 bb);

std::string print_bitboardForLog(u64 bb);

inline void logToFile(const std::string message) {
    static std::ofstream log("/home/simon3604/Chessbot/engine_log.txt", std::ios::app);

    if (log) {
        log << message << '\n';
    }
}


std::string printBoardAsLetters(const Board& board, bool forLog);

inline const u64* getTypeBB(int pos,const Board& board) {
    u64 mask = 1ULL << pos;
    for (int p = 0; p < 12; p++) {
        if (mask & board.pieces[p]) {
            return &board.pieces[p];
        }
    }
    return nullptr;
}





inline std::string numToPos(int num) {
    std::string str;
    u64 numBB = 1ULL << num;
    if (numBB & FILE_A) {
        str = "a";
    } else if (numBB & FILE_B) {
        str = "b";
    } else if (numBB & FILE_C) {
        str = "c";
    } else if (numBB & FILE_D) {
        str = "d";
    } else if (numBB & FILE_E) {
        str = "e";
    } else if (numBB & FILE_F) {
        str = "f";
    } else if (numBB & FILE_G) {
        str = "g";
    } else if (numBB & FILE_H) {
        str = "h";
    }
    if (numBB & RANK_1) {
        str += "1";
    } else if (numBB & RANK_2) {
        str += "2";
    } else if (numBB & RANK_3) {
        str += "3";
    } else if (numBB & RANK_4) {
        str += "4";
    } else if (numBB & RANK_5) {
        str += "5";
    } else if (numBB & RANK_6) {
        str += "6";
    } else if (numBB & RANK_7) {
        str += "7";
    } else if (numBB & RANK_8) {
        str += "8";
    }
    return str;
}

// FEN -> Board
Board fenUnloader(const std::string& fen);



int pieceValueAtSq(const Board& board, int sq);

bool has_non_pawn_material(Board& board, Color side);

Move parseUCIMove(const std::string& moveStr, Board& board);


void parsePositionCommand(const std::string& input, Board& board, Color side);

bool sameBoard(const Board& a, const Board& b);

Color getColor(const Board& board, int sq);


inline Piece getPieceType(const Board& board, int sq) {
    if (sq < 0 || sq > 63) return NONE;
    u64 mask = 1ULL << sq;
    if (mask & board.pieces[WQ]) return WQ;
    if (mask & board.pieces[WR]) return WR;
    if (mask & board.pieces[WB]) return WB;
    if (mask & board.pieces[WN]) return WN;
    if (mask & board.pieces[WP]) return WP;
    if (mask & board.pieces[WK]) return WK;
    if (mask & board.pieces[BQ]) return BQ;
    if (mask & board.pieces[BR]) return BR;
    if (mask & board.pieces[BB]) return BB;
    if (mask & board.pieces[BN]) return BN;
    if (mask & board.pieces[BP]) return BP;
    if (mask & board.pieces[BK]) return BK;
    return NONE;

     
}


u64 perft(Board& board, int ply, int depth, u64& captures, u64& promotions, 
    u64& castles, u64& enPassants, u64& checks, u64& checkmates);

inline int pieceValue(Piece p) {
    switch (p) {
        case WP: return PAWN_VALUE;
        case WN: return KNIGHT_VALUE;
        case WB: return BISHOP_VALUE;
        case WR: return ROOK_VALUE;
        case WQ: return QUEEN_VALUE;
        case WK: return KING_VALUE;
        case BP: return PAWN_VALUE;
        case BN: return KNIGHT_VALUE;
        case BB: return BISHOP_VALUE;
        case BR: return ROOK_VALUE;
        case BQ: return QUEEN_VALUE;
        case BK: return KING_VALUE;
        default: return 0;
    }
}

void initZobrist();

bool canEnPassant(const Board& board, Color side);

u64 computeHash(const Board& board, Color side);

char getPieceLetter(Piece piece);

bool hasPawnOnFile(const Board& board, int file, Color side);

int fileOf(int sq);

u64 attackers(const Board& board, Color side, int sq);

void hashXor(Board& board, u64& hash, u64 key, const std::string& label);



