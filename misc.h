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



inline u64 rand64() {
    static std::mt19937_64 rng(123456); // fixed seed (or use time)
    return rng();
}

void benchmarkSearch(Board board, int depth, Color side);

void print_bitboard(u64 bb);

std::string print_bitboardForLog(u64 bb);

inline void logToFile(const std::string message) {
    static std::ofstream log("/home/simon3604/Chessbot/engine_log.txt", std::ios::app);

    if (log) {
        log << message << '\n';
    }
}

void checkBoard(const Board& board);

std::string printBoardAsLetters(const Board& board, bool forLog);

inline const u64* getTypeBB(int pos,const Board& board) {
    u64 mask = 1ULL << pos;
    if (mask & board.pawns_white)  return &board.pawns_white;
    if (mask & board.rooks_white)  return &board.rooks_white;
    if (mask & board.knights_white) return &board.knights_white;
    if (mask & board.bishops_white) return &board.bishops_white;
    if (mask & board.queens_white)  return &board.queens_white;
    if (mask & board.king_white)   return &board.king_white;

    if (mask & board.pawns_black)  return &board.pawns_black;
    if (mask & board.rooks_black)  return &board.rooks_black;
    if (mask & board.knights_black) return &board.knights_black;
    if (mask & board.bishops_black) return &board.bishops_black;
    if (mask & board.queens_black)  return &board.queens_black;
    if (mask & board.king_black)   return &board.king_black;

    return nullptr;
}

inline u64* getTypeBB(int pos, Board& board) {
    u64 mask = 1ULL << pos;
    if (mask & board.pawns_white)  return &board.pawns_white;
    if (mask & board.rooks_white)  return &board.rooks_white;
    if (mask & board.knights_white) return &board.knights_white;
    if (mask & board.bishops_white) return &board.bishops_white;
    if (mask & board.queens_white)  return &board.queens_white;
    if (mask & board.king_white)   return &board.king_white;

    if (mask & board.pawns_black)  return &board.pawns_black;
    if (mask & board.rooks_black)  return &board.rooks_black;
    if (mask & board.knights_black) return &board.knights_black;
    if (mask & board.bishops_black) return &board.bishops_black;
    if (mask & board.queens_black)  return &board.queens_black;
    if (mask & board.king_black)   return &board.king_black;

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



std::string randomMove(std::vector<Move>& moves, Color side, Board& board);


std::string evalMove(std::vector<Move>& moves, Color side, Board& board);


int pieceValueAtSq(const Board& board, int sq);



Move parseUCIMove(const std::string& moveStr, Board& board);


void parsePositionCommand(const std::string& input, Board& board, Color side);

bool sameBoard(const Board& a, const Board& b);

Color getColor(const Board& board, int sq);


inline Piece getPieceType(const Board& board, int sq) {
    if (sq < 0 || sq > 63) return NONE;
    u64 mask = 1ULL << sq;
    if (mask & board.queens_white)  return QUEEN;
    if (mask & board.rooks_white)   return ROOK;
    if (mask & board.bishops_white) return BISHOP;
    if (mask & board.knights_white) return KNIGHT;
    if (mask & board.pawns_white)   return PAWN;
    if (mask & board.king_white)    return KING;
    if (mask & board.queens_black)  return QUEEN;
    if (mask & board.rooks_black)   return ROOK;
    if (mask & board.bishops_black) return BISHOP;
    if (mask & board.knights_black) return KNIGHT;
    if (mask & board.pawns_black)   return PAWN;
    if (mask & board.king_black)    return KING;
    return NONE;

     
}

inline u64* getBitboard(Board& board, Piece p, Color side) {
    if (side == WHITE) {
        switch (p) {
            case PAWN: return &board.pawns_white;
            case KNIGHT: return &board.knights_white;
            case BISHOP: return &board.bishops_white;
            case ROOK: return &board.rooks_white;
            case QUEEN: return &board.queens_white;
            case KING: return &board.king_white;
            default: return nullptr;
        }
    } else {
        switch (p) {
            case PAWN: return &board.pawns_black;
            case KNIGHT: return &board.knights_black;
            case BISHOP: return &board.bishops_black;
            case ROOK: return &board.rooks_black;
            case QUEEN: return &board.queens_black;
            case KING: return &board.king_black;
            default: return nullptr;
        }
    }
}

u64 perft(Board& board, int depth, Color side, u64& captures, u64& promotions, 
    u64& castles, u64& enPassants, u64& checks, u64& checkmates);

inline int pieceValue(Piece p) {
    switch (p) {
        case PAWN: return 100;
        case KNIGHT: return 300;
        case BISHOP: return 300;
        case ROOK: return 500;
        case QUEEN: return 900;
        case KING: return 10000;
        default: return 0;
    }
}

void initZobrist();

u64 computeHash(const Board& board, Color side);

char getPieceLetter(Piece piece);

bool hasPawnOnFile(const Board& board, int file, Color side);

int fileOf(int sq);

u64 attackers(const Board& board, Color side, int sq);

