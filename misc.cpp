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
#include "misc.h"
#include "makeMove.h"
#include "search.h"
#include <chrono>
#include <random>
#include "movePicker.h"

std::mt19937_64 rng;  // global RNG

void seedRNG(uint64_t seed) {
    rng.seed(seed);  // deterministic default for debugging
}

void benchmarkSearch(Board board, int depth, Color side)
{

    auto start = std::chrono::high_resolution_clock::now();


    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now(); 

    GoParams go = {};
    go.depth = depth;

    Move bestMove = search(board, go);

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




std::string printBoardAsLetters(const Board& board, bool forLog) {
    if (forLog == false) {
        for (int rank = 7; rank >= 0; --rank) { // print from rank 8 to 1
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                char c = '.';

                u64 mask = 1ULL << sq;

                if (mask & board.pieces[WP])   c = 'P';
                else if (mask & board.pieces[WN]) c = 'N';
                else if (mask & board.pieces[WB]) c = 'B';
                else if (mask & board.pieces[WR])   c = 'R';
                else if (mask & board.pieces[WQ])  c = 'Q';
                else if (mask & board.pieces[WK])    c = 'K';
                else if (mask & board.pieces[BP])   c = 'p';
                else if (mask & board.pieces[BN]) c = 'n';
                else if (mask & board.pieces[BB]) c = 'b';
                else if (mask & board.pieces[BR])   c = 'r';
                else if (mask & board.pieces[BQ])  c = 'q';
                else if (mask & board.pieces[BK])    c = 'k';

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

                if (mask & board.pieces[WP])   c = 'P';
                else if (mask & board.pieces[WN]) c = 'N';
                else if (mask & board.pieces[WB]) c = 'B';
                else if (mask & board.pieces[WR])   c = 'R';
                else if (mask & board.pieces[WQ])  c = 'Q';
                else if (mask & board.pieces[WK])    c = 'K';
                else if (mask & board.pieces[BP])   c = 'p';
                else if (mask & board.pieces[BN]) c = 'n';
                else if (mask & board.pieces[BB]) c = 'b';
                else if (mask & board.pieces[BR])   c = 'r';
                else if (mask & board.pieces[BQ])  c = 'q';
                else if (mask & board.pieces[BK])    c = 'k';

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
    board.pieces[WP] = 0;
    board.pieces[WN] = 0;
    board.pieces[WB] = 0;
    board.pieces[WR] = 0;
    board.pieces[WQ] = 0;
    board.pieces[WK] = 0;

    board.pieces[BP] = 0;
    board.pieces[BN] = 0;
    board.pieces[BB] = 0;
    board.pieces[BR] = 0;
    board.pieces[BQ] = 0;
    board.pieces[BK] = 0;

    board.all_white = 0;
    board.all_black = 0;

    std::istringstream ss(fen);

    std::string placement, stm, castling, ep;
    int halfmove, fullmove;

    ss >> placement >> stm >> castling >> ep >> halfmove >> fullmove;

    board.halfmoveClock = halfmove;
    board.fullmoveNumber = fullmove;
    
    
    for (char c : placement) {
        if (c == ' ') break;
        else if (c == '/') { square -= 16; }
        else if (isdigit(c)) { square += c - '0'; }
        else {
            u64 bit = 1ULL << square;
            switch(c) {
                case 'P': 
                    board.pieces[WP]   |= bit; 
                    board.all_white |= bit;
                    break;
                case 'N': 
                    board.pieces[WN] |= bit; 
                    board.all_white |= bit;
                    break;
                case 'B': 
                    board.pieces[WB] |= bit; 
                    board.all_white |= bit;
                    break;
                case 'R': 
                    board.pieces[WR]   |= bit; 
                    board.all_white |= bit;
                    break;
                case 'Q': 
                    board.pieces[WQ]  |= bit; 
                    board.all_white |= bit;
                    break;
                case 'K': 
                    board.pieces[WK] |= bit; 
                    board.all_white |= bit;
                    break;
                case 'p': 
                    board.pieces[BP]   |= bit; 
                    board.all_black |= bit;
                    break;
                case 'n': 
                    board.pieces[BN] |= bit; 
                    board.all_black |= bit;
                    break;
                case 'b': 
                    board.pieces[BB] |= bit; 
                    board.all_black |= bit;
                    break;
                case 'r': 
                    board.pieces[BR]   |= bit; 
                    board.all_black |= bit;
                    break;
                case 'q': 
                    board.pieces[BQ]  |= bit; 
                    board.all_black |= bit;
                    break;
                case 'k': 
                    board.pieces[BK]    |= bit; 
                    board.all_black |= bit;
                    break;
            }
            square++;
        }
    }

    if (stm == "w") {
        board.sideToMove = WHITE;
    } else {
        board.sideToMove = BLACK;
    }
    
    
    board.castlingRights = 0;

    if (castling.find('K') != std::string::npos) board.castlingRights |= CWK;
    if (castling.find('Q') != std::string::npos) board.castlingRights |= CWQ;
    if (castling.find('k') != std::string::npos) board.castlingRights |= CBK;
    if (castling.find('q') != std::string::npos) board.castlingRights |= CBQ;
    


    if (ep != "-") {
        int file = ep[0] - 'a';
        int rank = ep[1] - '1';
        board.enPassantSquare = rank * 8 + file;
    } else {
        board.enPassantSquare = -1;
    }
    
    board.hash = 0;
    board.hash = computeHash(board, board.sideToMove);

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
    if (mask & board.pieces[WQ])  return QUEEN_VALUE;
    if (mask & board.pieces[WR])   return ROOK_VALUE;
    if (mask & board.pieces[WB]) return BISHOP_VALUE;
    if (mask & board.pieces[WN]) return KNIGHT_VALUE;
    if (mask & board.pieces[WP])   return PAWN_VALUE;
    if (mask & board.pieces[WK])    return KING_VALUE;
    if (mask & board.pieces[BQ])  return QUEEN_VALUE;
    if (mask & board.pieces[BR])   return ROOK_VALUE;
    if (mask & board.pieces[BB]) return BISHOP_VALUE;
    if (mask & board.pieces[BN]) return KNIGHT_VALUE;
    if (mask & board.pieces[BP])   return PAWN_VALUE;
    if (mask & board.pieces[BK])    return KING_VALUE;
    return 0;
}


bool has_non_pawn_material(Board& board, Color side) {
    if (side == WHITE) {
        if (board.all_white & ~board.pieces[WP] & ~board.pieces[WK]) return true;
    } else if (side == WHITE) {
        if (board.all_black & ~board.pieces[BP] & ~board.pieces[BK]) return true;
    }
    return false;
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
    
        if (board.sideToMove == WHITE) {
            switch (moveStr[4]) {
                case 'q': promotion = WQ; break;
                case 'r': promotion = WR;  break;
                case 'b': promotion = WB; break;
                case 'n': promotion = WN; break;
            }
        } else if (board.sideToMove == BLACK) {
            switch (moveStr[4]) {
                case 'q': promotion = BQ; break;
                case 'r': promotion = BR;  break;
                case 'b': promotion = BB; break;
                case 'n': promotion = BN; break;
            }
        }
        
        
    }

    //Handle captures
    Piece captured = NONE;
    u64 opp = (board.sideToMove == WHITE) ? board.all_black : board.all_white;
    if (to == board.enPassantSquare) {
        int epCapture = (board.sideToMove == WHITE) ? to - 8 : to + 8;
        captured = getPieceType(board, epCapture);
    } else if ((1ULL << to) & opp) {
        captured = getPieceType(board, to);
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
    move.captured = captured;
    move.promotion = promotion;
    return move;
}


void parsePositionCommand(const std::string& input, Board& board, Color side) {
  


    std::istringstream iss(input);

    std::string token;
    iss >> token;

    std::string type;
    iss >> type;

    if (type == "startpos") {
        board = fenUnloader(startFen);


        
    } 
    else if (type == "fen") {
        std::string word;
        std::string fen;

        for (int i = 0; i < 6 && iss >> word; ++i) {
            fen += word + " ";
        }



        board = fenUnloader(fen);


        

       

    }

    // Now check for optional "moves"
    std::string movesKeyword;
    if (iss >> movesKeyword && movesKeyword == "moves") {
        std::string moveStr;
        while (iss >> moveStr) {
            Move move = parseUCIMove(moveStr, board);
            makeMove(move, board);
        
        }
    }

    
    
}

bool sameBoard(const Board& a, const Board& b)
{
    bool same = true;
        if (a.pieces[BP]   != b.pieces[WP]) {
            std::cout << "Undo -> Pawns";
           
            same = false;
        }   
        if (a.pieces[WN]   != b.pieces[WN]) {
            std::cout << "Undo -> Knights";
            same = false;
        }
        if (a.pieces[WN]   != b.pieces[WN]) {
            std::cout << "Undo -> Bishops";
            same = false;
        }
        if (a.pieces[WR]   != b.pieces[WR]) {
            std::cout << "Undo -> Rooks";
            same = false;
        }
        if (a.pieces[WQ]   != b.pieces[WQ]) {
            std::cout << "Undo -> Queens";
            same = false;
        }
        if (a.pieces[WK]   != b.pieces[WK]) {
            std::cout << "Undo -> King";
            same = false;
        }

        if (a.pieces[BP] != b.pieces[BP]) {
            std::cout << "Undo -> Pawns";
            
            same = false;
        }   
        if (a.pieces[BN]   != b.pieces[BN]) {
            std::cout << "Undo -> Knights";
            same = false;
        }
        if (a.pieces[BB]   != b.pieces[BB]) {
            std::cout << "Undo -> Bishops";
            same = false;
        }
        if (a.pieces[BR]  != b.pieces[BR]) {
            std::cout << "Undo -> Rooks";
            same = false;
        }
        if (a.pieces[BQ]  != b.pieces[BQ]) {
            std::cout << "Undo -> Queens";
            same = false;
        }
        if (a.pieces[BK]  != b.pieces[BK]) {
            std::cout << "Undo -> King";
            same = false;
        }

        if (a.enPassantSquare != b.enPassantSquare) {
            std::cout << "Undo -> Ep";
            same = false;
        }
        if (a.castlingRights != b.castlingRights) {
            std::cout << "Undo -> Castlingrights";
            same = false;
        }
        if (a.halfmoveClock != b.halfmoveClock) {
            std::cout << "Undo -> HalfMoveClock";
            same = false;
        }
        if (a.fullmoveNumber != b.fullmoveNumber) {
            std::cout << "Undo -> fullMoveNumber";
            same = false;
        }
        if (a.all_black != b.all_black) {
            std::cout << "Undo -> all_black";
            same = false;
        }
        if (a.all_white != b.all_white) {
            print_bitboard(a.all_white);
            print_bitboard(b.all_white);
            std::cout << "Undo -> all_white";
            same = false;
        }
        if (a.hash != b.hash) {
            std::cout << "Undo -> hash";
            same = false;
        }
        return same;
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


u64 perft(Board& board, int ply, int depth, u64& captures, u64& promotions, u64& castles, u64& enPassants, u64& checks, u64& checkmates) {
    if (depth == 0)
        return 1;

    Color side = board.sideToMove;

    Move* moves = moveStack[ply];
    int count = generateLegalMoves(board, side, moves);
    Color opp = (side == WHITE) ? BLACK : WHITE;

    u64 nodes = 0;
    u64 total = 0;
    

    for (int i = 0; i < count; i++) {
        Move m = moves[i];
        Undo u = makeMove(m, board);

        if (depth == 1) {
            if (m.captured != NONE || (m.flags & ENPASSANT)) captures++;
            if (m.promotion != NONE) promotions++;
            if (m.from2 != -1) castles++;
            if (m.flags & ENPASSANT) enPassants++;
            bool inCheck = isKingInCheck(opp, board);
            if (inCheck) {
                checks++;
                Move* oppMoves = moveStack[ply + 1];
                int oppCount = generateLegalMoves(board, opp, oppMoves);
                if (oppCount == 0) checkmates++;
            }

        }
        
        
        nodes = perft(board, ply + 1, depth - 1, captures, promotions, castles, enPassants, checks, checkmates);
        total += nodes;

        undoMove(m, board, u);
        if (depth == 3) {
            std::cout << numToPos(m.from) << numToPos(m.to) << ": " << nodes << "\n";
    
        }
    
    }

    return total;
}

void initZobrist() {
   
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    // pieces

    for (int p = 0; p < 12; p++) {
        for (int sq = 0; sq < 64; sq++) {
            zobrist[p][sq] = dist(rng);
        }
    }
    


    // side
    zobristSide = dist(rng);

    // castling
    for (int i = 0; i < 16; i++)
        zobristCastling[i] = dist(rng);

    // en passant
    for (int i = 0; i < 8; i++)
        zobristEnPassant[i] = dist(rng);
}


bool canEnPassant(const Board& board, Color side) {

    int epSq = board.enPassantSquare;
    u64 pawns = (side == WHITE) ? board.pieces[WP] : board.pieces[BP];
    int fromEp;

    if (side == WHITE) {
        if (epSq % 8 != 0) {  // left EP
            fromEp = epSq - 9;
            if ((fromEp >= 0 && fromEp < 64) && (pawns & (1ULL << fromEp))) {
                return true;
            }
        }
        if (epSq % 8 != 7) {  // right EP
            fromEp = epSq - 7;
            if (fromEp >= 0 && fromEp < 64 && (pawns & (1ULL << fromEp))) {
                return true;            
            }
        }
    } else {  // black
        if (epSq % 8 != 0) {  // left EP
            fromEp = epSq + 7;
            if (fromEp >= 0 && fromEp < 64 && (pawns & (1ULL << fromEp))) { 
                return true;
            }
        }
        if (epSq % 8 != 7) {  // right EP
            fromEp = epSq + 9;
            if (fromEp >= 0 && fromEp < 64 && (pawns & (1ULL << fromEp))) {
                return true;
            }
        }
    }
    return false;
}

u64 computeHash(const Board& board, Color side) {
    u64 hash = 0;

    // Pieces
    for (int sq = 0; sq < 64; sq++) {
        Piece p = getPieceType(board, sq);
        if (p != NONE) {
            
            hash ^= zobrist[p][sq];
        }
    }

    // Side to move
    if (board.sideToMove == BLACK)
        hash ^= zobristSide;

    // Castling rights
    hash ^= zobristCastling[board.castlingRights];

    if (board.enPassantSquare != -1) {
        int file = board.enPassantSquare % 8;
        hash ^= zobristEnPassant[file];
    }

    return hash;
}

char getPieceLetter(Piece piece) {
    switch (piece)
    {
    case WQ:
        return 'q';

    case WR:
        return 'r';

    case WB:
        return 'b';

    case WN:
        return 'n';
    case BQ:
        return 'q';

    case BR:
        return 'r';

    case BB:
        return 'b';

    case BN:
        return 'n';


    
    }
    return 'q';
}

bool hasPawnOnFile(const Board& board, int file, Color side) {
    
    u64 fileBB;
    switch (file)
    {
    case 0:
        fileBB = FILE_A;
        break;
    
    case 1:
        fileBB = FILE_B;
        break;
    
    case 2:
        fileBB = FILE_C;
        break;
    
    case 3:
        fileBB = FILE_D;
        break;
    
    case 4:
        fileBB = FILE_E;
        break;
    
    case 5:
        fileBB = FILE_F;
        break;
    
    case 6:
        fileBB = FILE_G;
        break;
    
    case 7:
        fileBB = FILE_H;
        break;

    default: 
        std::cerr << "hasPawnOnFile: No file matching" << std::endl;
        std::cerr << "File: " << file << std::endl;
    
    }


    if (side == WHITE) {
        if (fileBB & board.pieces[WP]) {
            return true;
        }
    } else if (side == BLACK) {
        if (fileBB & board.pieces[BP]) {
            return true;
        }
    }
    return false;
}

int fileOf(int sq) {
    u64 sqBB = 1ULL << sq;

    if (sqBB & FILE_A) return 0;
    if (sqBB & FILE_B) return 1;
    if (sqBB & FILE_C) return 2;
    if (sqBB & FILE_D) return 3;
    if (sqBB & FILE_E) return 4;
    if (sqBB & FILE_F) return 5;
    if (sqBB & FILE_G) return 6;
    if (sqBB & FILE_H) return 7;
    
    std::cerr << "fileOf: No matching file" << std::endl;
    std::cerr << "Square: " << sq << std::endl;
    return 0;
}

u64 attackers(const Board& board, Color side, int sq) {
    u64 attackers = 0ULL;
    u64 sqBB = 1ULL << sq;
    u64 occ = board.all_white | board.all_black;

    if (side == WHITE) {
        //PAWNS
        
        if ((sqBB >> 7) & ~FILE_A & board.pieces[WP]) {
            if ((sq - 7) >= 0)
            attackers &= 1ULL >> sq - 7;
        }
        if ((sqBB >> 9) & ~FILE_H & board.pieces[BP]) {
            if ((sq - 9) >= 0)
            attackers &= 1ULL >> sq - 9;
        }



        //KNIGHTS
        u64 knights = knightAttacks[sq] & board.pieces[WN];

        while (knights) {

            attackers |= lsb(knights); 
            knights &= knights -1;
        }


        //KING
        u64 king = kingAttacks[sq] & board.pieces[WK];

        while (king) {

            attackers |= lsb(king); 
            king &= king -1;
        }

        //BISHOPS
        u64 bishops = getBishopAttackMagics(sq, occ) & board.pieces[WB];

        while (bishops) {

            attackers |= lsb(bishops); 
            bishops &= bishops -1;
        }

        //ROOKS
        u64 rooks = getRookAttackMagics(sq, occ) & board.pieces[WR];

        while (rooks) {

            attackers |= lsb(rooks); 
            rooks &= rooks -1;
        }

        //QUEENS
        u64 queens = getBishopAttackMagics(sq, occ) & getRookAttackMagics(sq, occ) & board.pieces[WQ];

        while (queens) {

            attackers |= lsb(queens);
            queens &= queens -1;
        }
    } else if (side == BLACK) {
        
        //PAWNS
        
        if ((sqBB << 7) & ~FILE_H & board.pieces[WP]) {
            if ((sq + 7) <= 63)
            attackers |= 1ULL >> sq + 7;
        }
        if ((sqBB << 9) & ~FILE_A & board.pieces[BP]) {
            if ((sq + 9) <= 63)
            attackers |= 1ULL >> sq + 9;
        }

        //KNIGHTS
        u64 knights = knightAttacks[sq] & board.pieces[BN];

        while (knights) {

            attackers |= lsb(knights); 
            knights &= knights -1;
        }


        //KING
        u64 king = kingAttacks[sq] & board.pieces[BK];

        while (king) {

            attackers |= lsb(king); 
            king &= king -1;
        }

        //BISHOPS
        u64 bishops = getBishopAttackMagics(sq, occ) & board.pieces[BB];

        while (bishops) {

            attackers |= lsb(bishops); 
            bishops &= bishops -1;
        }

        //ROOKS
        u64 rooks = getRookAttackMagics(sq, occ) & board.pieces[BR];

        while (rooks) {

            attackers |= lsb(rooks); 
            rooks &= rooks -1;
        }

        //QUEENS
        u64 queens = getBishopAttackMagics(sq, occ) & getRookAttackMagics(sq, occ) & board.pieces[BQ];

        while (queens) {

            attackers |= lsb(queens);
            queens &= queens -1;
        }
    
    }

    return attackers;

}

void hashXor(Board& board, u64& hash, u64 key, const std::string& label) {
    std::cout << label << " XOR " << key << "hash: " << hash << "\n";
    hash ^= key;

    std::cout << "Correct hash (white): " << computeHash(board, board.sideToMove) << "\n";
}
