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
#include "misc.h"

void print_bitboard(u64 bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            std::cerr << ((bb >> square) & 1ULL) << " ";
        }
        std::cerr << "\n";
    }
    std::cerr << "\n";
}

void logToFile(const std::string message) {
    std::ofstream log("/home/simon3604/Chessbot/engine_log.txt", std::ios::app); // "app" = append mode
    if (log.is_open()) {
    
        log << message << std::endl;
    }
}


std::string printBoardAsLetters(const Board& board, bool forLog) {
    if (forLog == false) {
        for (int rank = 7; rank >= 0; --rank) { // print from rank 8 to 1
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                char c = '.';

                u64 mask = 1ULL << sq;

                if (mask & board.pawns_white)   c = 'P';
                else if (mask & board.knights_white) c = 'N';
                else if (mask & board.bishops_white) c = 'B';
                else if (mask & board.rooks_white)   c = 'R';
                else if (mask & board.queens_white)  c = 'Q';
                else if (mask & board.king_white)    c = 'K';
                else if (mask & board.pawns_black)   c = 'p';
                else if (mask & board.knights_black) c = 'n';
                else if (mask & board.bishops_black) c = 'b';
                else if (mask & board.rooks_black)   c = 'r';
                else if (mask & board.queens_black)  c = 'q';
                else if (mask & board.king_black)    c = 'k';

                std::cerr << c << " ";
            }
            std::cerr << "\n";
        }
        return "noreturn";
    } else {
        std::string log;
        for (int rank = 7; rank >= 0; --rank) { // print from rank 8 to 1
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                char c = '.';

                u64 mask = 1ULL << sq;

                if (mask & board.pawns_white)   c = 'P';
                else if (mask & board.knights_white) c = 'N';
                else if (mask & board.bishops_white) c = 'B';
                else if (mask & board.rooks_white)   c = 'R';
                else if (mask & board.queens_white)  c = 'Q';
                else if (mask & board.king_white)    c = 'K';
                else if (mask & board.pawns_black)   c = 'p';
                else if (mask & board.knights_black) c = 'n';
                else if (mask & board.bishops_black) c = 'b';
                else if (mask & board.rooks_black)   c = 'r';
                else if (mask & board.queens_black)  c = 'q';
                else if (mask & board.king_black)    c = 'k';

                log += c;
                log += " ";
            }
            log += "\n";
        }
        return log;
    }

}

