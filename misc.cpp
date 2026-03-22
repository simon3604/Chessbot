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
#include "search.h"
#include <chrono>

void benchmarkSearch(Board board, int depth, Color side)
{

    auto start = std::chrono::high_resolution_clock::now();

    // Instead of search(), call your findBestMove function
    Move bestMove = findBestMove(board, side, depth);

    auto end = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();

    std::cerr << "=== Search Benchmark ===\n";
    std::cerr << "Depth: " << depth << "\n";
    std::cerr << "Nodes: " << nodes << "\n";   // nodes counted during alphaBeta
    std::cerr << "Time: " << seconds << " s\n";
    std::cerr << "NPS: " << (u64)(nodes / seconds) << "\n";
}

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




std::string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";




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

    //castling

    int from2 = -1;
    int to2 = -1;

    if (from == 4 && to == 6) { // white O-O
        from2 = 7;
        to2 = 5;
    }
    else if (from == 4 && to == 2) { // white O-O-O
        from2 = 0;
        to2 = 3;
    }
    else if (from == 60 && to == 62) { // black O-O
        from2 = 63;
        to2 = 61;
    }
    else if (from == 60 && to == 58) { // black O-O-O
        from2 = 56;
        to2 = 59;
    }

    // Construct the Move
    Move move;
    move.from = from;
    move.to = to;
    move.from2 = from2;
    move.to2 = to2;
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
    
        if (a.pawns_white   != b.pawns_white) {
            std::cerr << "Undo -> Pawns";
            return false;
        }   
        if (a.knights_white   != b.knights_white) {
            std::cerr << "Undo -> Knights";
            return false;
        }
        if (a.bishops_white   != b.bishops_white) {
            std::cerr << "Undo -> Bishops";
            return false;
        }
        if (a.rooks_white   != b.rooks_white) {
            std::cerr << "Undo -> Rooks";
            return false;
        }
        if (a.queens_white   != b.queens_white) {
            std::cerr << "Undo -> Queens";
            return false;
        }
        if (a.king_white   != b.king_white) {
            std::cerr << "Undo -> King";
            return false;
        }

        if (a.pawns_black != b.pawns_black) {
            std::cerr << "Undo -> Pawns";
            return false;
        }   
        if (a.knights_black   != b.knights_black) {
            std::cerr << "Undo -> Knights";
            return false;
        }
        if (a.bishops_black   != b.bishops_black) {
            std::cerr << "Undo -> Bishops";
            return false;
        }
        if (a.rooks_black  != b.rooks_black) {
            std::cerr << "Undo -> Rooks";
            return false;
        }
        if (a.queens_black  != b.queens_black) {
            std::cerr << "Undo -> Queens";
            return false;
        }
        if (a.king_black   != b.king_black) {
            std::cerr << "Undo -> King";
            return false;
        }

        if (a.enPassantSquare != b.enPassantSquare) {
            std::cerr << "Undo -> Ep";
            return false;
        }
        return true;
}

Color getColor(const Board& board, int sq) {
    u64 sqBB = 1ULL << sq;

    if (sqBB & board.all_white) {
        return WHITE;
    } 
    else {
        return BLACK;
    }
}


u64 perft(Board& board, int depth, Color side) {
    if (depth == 0)
        return 1;

    std::vector<Move> moves;
    generateLegalMoves(board, side, moves);

    u64 nodes = 0;

    for (Move m : moves) {
        Undo u = makeMove(m, board, side);

        nodes += perft(board, depth - 1, (side == WHITE) ? BLACK : WHITE);

        undoMove(m, board, side, u);
    }

    return nodes;
}

void initZobrist() {
    for (int c = 0; c < 2; c++)
        for (int p = 0; p < 6; p++)
            for (int sq = 0; sq < 64; sq++)
                zobrist[c][p][sq] = rand64();

    zobristSide = rand64();
}

u64 computeHash(const Board& board, Color side) {
    u64 hash = 0;

    for (int sq = 0; sq < 64; sq++) {
        Piece p = getPieceType(board, sq);
        if (p != NONE) {
            Color c = getColor(board, sq);
            hash ^= zobrist[c][p][sq];
        }
    }

    if (side == BLACK)
        hash ^= zobristSide;

    return hash;
}







