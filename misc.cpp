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
#include "makeMove.h"

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

std::string print_bitboardForLog(u64 bb) {
    std::string log;
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            log += ((bb >> square) & 1ULL) ? '1' : '0';
            log += " ";
        }
        log += "\n";
    }
    log += "\n";
    return log;
}

void logToFile(const std::string message) {
    static std::ofstream log("/home/simon3604/Chessbot/engine_log.txt", std::ios::app);

    if (log) {
        log << message << '\n';
    }
}

void checkBoard(const Board& board) {
    u64 bb = board.king_white; 
    logToFile(print_bitboardForLog(bb));
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

const u64* getTypeBB(int pos,const Board& board) {
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

u64* getTypeBB(int pos, Board& board) {
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
// void check_board_integrity(const Board &board, int square) {
//     u64 all = 0ULL;
//     auto check_and_report = [&](u64 bb, const char* name) {
//         if (bb & all) {
//             std::cerr << square << prefix << "OVERLAP detected for " << name << "\n";
//         }
//         all |= bb;
//     };

//     check_and_report(board.pawns_white,   "pawns_white");
//     check_and_report(board.knights_white, "knights_white");
//     check_and_report(board.bishops_white, "bishops_white");
//     check_and_report(board.rooks_white,   "rooks_white");
//     check_and_report(board.queens_white,  "queens_white");
//     check_and_report(board.king_white,    "king_white");

//     check_and_report(board.pawns_black,   "pawns_black");
//     check_and_report(board.knights_black, "knights_black");
//     check_and_report(board.bishops_black, "bishops_black");
//     check_and_report(board.rooks_black,   "rooks_black");
//     check_and_report(board.queens_black,  "queens_black");
//     check_and_report(board.king_black,    "king_black");

//     // check all_white/all_black are consistent:
//     u64 recomputed_white = board.pawns_white | board.knights_white | board.bishops_white |
//                            board.rooks_white  | board.queens_white  | board.king_white;
//     u64 recomputed_black = board.pawns_black | board.knights_black | board.bishops_black |
//                            board.rooks_black  | board.queens_black  | board.king_black;

   
//     // check bit 0 specifically
//     // bool bit0set = ((board.queens_white | board.rooks_white | board.pawns_white |
//     //                  board.knights_white | board.bishops_white) & 1ULL) != 0;
//     // if (bit0set) {
//     //     std::cerr << prefix << "bit 0 is set in white piece bitboards!\n";
//     //     print_bitboard(board.queens_white);
//     //     print_bitboard(board.rooks_white);
//     //     print_bitboard(board.pawns_white);
//     // }
// }

std::string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";



std::string numToPos(int num) {
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
Board fenUnloader(const std::string& fen) {
    int square = 56;
    Board board = {};
    board.pawns_white = 0;
    board.knights_white = 0;
    board.bishops_white = 0;
    board.rooks_white = 0;
    board.queens_white = 0;
    board.king_white = 0;

    board.pawns_black = 0;
    board.knights_black = 0;
    board.bishops_black = 0;
    board.rooks_black = 0;
    board.queens_black = 0;
    board.king_black = 0;

    board.all_white = 0;
    board.all_black = 0;
    for (char c : fen) {
        if (c == ' ') break;
        else if (c == '/') { square -= 16; }
        else if (isdigit(c)) { square += c - '0'; }
        else {
            u64 bit = 1ULL << square;
            switch(c) {
                case 'P': 
                    board.pawns_white   |= bit; 
                    board.all_white |= bit;
                    break;
                case 'N': 
                    board.knights_white |= bit; 
                    board.all_white |= bit;
                    break;
                case 'B': 
                    board.bishops_white |= bit; 
                    board.all_white |= bit;
                    break;
                case 'R': 
                    board.rooks_white   |= bit; 
                    board.all_white |= bit;
                    break;
                case 'Q': 
                    board.queens_white  |= bit; 
                    board.all_white |= bit;
                    break;
                case 'K': 
                    board.king_white    |= bit; 
                    board.all_white |= bit;
                    break;
                case 'p': 
                    board.pawns_black   |= bit; 
                    board.all_black |= bit;
                    break;
                case 'n': 
                    board.knights_black |= bit; 
                    board.all_black |= bit;
                    break;
                case 'b': 
                    board.bishops_black |= bit; 
                    board.all_black |= bit;
                    break;
                case 'r': 
                    board.rooks_black   |= bit; 
                    board.all_black |= bit;
                    break;
                case 'q': 
                    board.queens_black  |= bit; 
                    board.all_black |= bit;
                    break;
                case 'k': 
                    board.king_black    |= bit; 
                    board.all_black |= bit;
                    break;
            }
            square++;
        }
    }
    return board;
}



// std::string randomMove(std::vector<Move>& moves, Color side, Board& board) {
    
//     if (moves.empty()) return "no moves";

//     int length = moves.size();

//     int idx = rand() % length;
//     Move m = moves[idx];

//     std::string selectedMove = "from " + std::to_string(m.from) + " to " + std::to_string(m.to);


//     u64* pieceBB = getTypeBB(m.from, board);
//     if (!pieceBB) {
//         return selectedMove + " (no piece at from)";
//     }

//     movePiece(*pieceBB, m.from, m.to, side, board); 

//     board.all_white = 
//         board.pawns_white | board.knights_white | board.bishops_white |
//         board.rooks_white | board.queens_white | board.king_white;
//     board.all_black = 
//         board.pawns_black | board.knights_black | board.bishops_black |
//         board.rooks_black | board.queens_black | board.king_black;

//     return selectedMove;
// } 



int pieceValueAtSq(const Board& board, int sq) {
    if (sq < 0 || sq > 63) return 0;
    u64 mask = 1ULL << sq;
    if (mask & board.queens_white)  return QUEEN_VALUE;
    if (mask & board.rooks_white)   return ROOK_VALUE;
    if (mask & board.bishops_white) return BISHOP_VALUE;
    if (mask & board.knights_white) return KNIGHT_VALUE;
    if (mask & board.pawns_white)   return PAWN_VALUE;
    if (mask & board.king_white)    return KING_VALUE;
    if (mask & board.queens_black)  return QUEEN_VALUE;
    if (mask & board.rooks_black)   return ROOK_VALUE;
    if (mask & board.bishops_black) return BISHOP_VALUE;
    if (mask & board.knights_black) return KNIGHT_VALUE;
    if (mask & board.pawns_black)   return PAWN_VALUE;
    if (mask & board.king_black)    return KING_VALUE;
    return 0;
}



Move parseUCIMove(const std::string& moveStr, Board& board) {
    // Defensive check
    if (moveStr.size() < 4) {
        std::cerr << "Invalid UCI move: " << moveStr << "\n";
        logToFile("Invalid UCI move: " + moveStr);

        return {};
    }

    // Convert UCI notation (e.g. "e2e4") into 0–63 square indices
    int fromFile = moveStr[0] - 'a';   // 'a' → 0, 'b' → 1, ...
    int fromRank = moveStr[1] - '1';   // '1' → 0, '2' → 1, ...
    int toFile   = moveStr[2] - 'a';
    int toRank   = moveStr[3] - '1';

    int from = fromRank * 8 + fromFile;
    int to   = toRank * 8 + toFile;

    // Handle optional promotion piece (e.g. "q", "r", "b", "n")
    Piece promotion = NONE;
    if (moveStr.size() == 5) {
        switch (moveStr[4]) {
            case 'q': promotion = QUEEN; break;
            case 'r': promotion = ROOK;  break;
            case 'b': promotion = BISHOP; break;
            case 'n': promotion = KNIGHT; break;
        }
    }

    // Construct the Move
    Move move;
    move.from = from;
    move.to = to;
    move.promotion = promotion;
    return move;
}


void parsePositionCommand(const std::string& input, Board& board, Color side) {
    std::istringstream iss(input);
    std::string token;
    iss >> token; // "position"



    std::string type;
    iss >> type; // "startpos" or "fen"

    if (type == "startpos") {
        board = fenUnloader(startFen);


        sideToMove = WHITE;
        
    } 
    else if (type == "fen") {
        std::string fenPart, fen;
        for (int i = 0; i < 6 && iss >> fenPart; ++i) {
            fen += fenPart + " ";
        }
        board = fenUnloader(fen);
        
        // get the side-to-move field
        std::istringstream fenStream(fen);
        std::string placement, stm;
        fenStream >> placement >> stm;
        sideToMove = (stm == "w") ? WHITE : BLACK;
    
    }

    // Now check for optional "moves"
    std::string movesKeyword;
    if (iss >> movesKeyword && movesKeyword == "moves") {
        std::string moveStr;
        while (iss >> moveStr) {
            Move move = parseUCIMove(moveStr, board);
            makeMove(move, board, sideToMove);
            if (sideToMove == WHITE) {
                sideToMove = BLACK;
            } else {
                sideToMove = WHITE;
            }
        }
    }

    
    
   
}

bool sameBoard(const Board& a, const Board& b)
{
    return
        a.pawns_white   == b.pawns_white   &&
        a.knights_white == b.knights_white &&
        a.bishops_white == b.bishops_white &&
        a.rooks_white   == b.rooks_white   &&
        a.queens_white  == b.queens_white  &&
        a.king_white    == b.king_white    &&

        a.pawns_black   == b.pawns_black   &&
        a.knights_black == b.knights_black &&
        a.bishops_black == b.bishops_black &&
        a.rooks_black   == b.rooks_black   &&
        a.queens_black  == b.queens_black  &&
        a.king_black    == b.king_black    &&

        a.enPassantSquare == b.enPassantSquare;
}

Piece getPieceType(const Board& board, int sq) {
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