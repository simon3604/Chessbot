#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>
#include "evaluation.h"
#include "moveGeneration.h"
#include "search.h"
#include "constants.h"
#include "misc.h"


using u64 = uint64_t;



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





void undoMoveEval(u64 &pieceBB, int fromSq, int toSq, Color side, Board &board, u64 capturedPiece) {
    // Move piece back
    pieceBB &= ~(1ULL << toSq);
    pieceBB |=  (1ULL << fromSq);
   
    // Restore captured piece
    if (capturedPiece) {
        u64* captureBB = getTypeBB(lsb(capturedPiece), board);
        if (captureBB) *captureBB |= capturedPiece;
    }

    // Update all_white / all_black
    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white | board.queens_white | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black | board.queens_black | board.king_black;
}








std::string randomMove(std::vector<Move>& moves, Color side, Board& board) {
    
    if (moves.empty()) return "no moves";

    int length = moves.size();

    int idx = rand() % length;
    Move m = moves[idx];

    std::string selectedMove = "from " + std::to_string(m.from) + " to " + std::to_string(m.to);


    u64* pieceBB = getTypeBB(m.from, board);
    if (!pieceBB) {
        return selectedMove + " (no piece at from)";
    }

    movePiece(*pieceBB, m.from, m.to, side, board); 

    board.all_white = 
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black = 
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    return selectedMove;
} 


std::string evalMove(std::vector<Move>& moves, Color side, Board& board) {
    if (moves.empty()) return "no moves";
    Board boardCopy = board;

    int length = moves.size();
    int bestMoveEval = 0;
    int from;
    int to;
    std::cerr << bestMoveEval << std::endl;
    static int lastFrom = -1, lastTo = -1;


    //start
    for (int i = 0; i < length; i++) {
        Move m = moves[i];
        u64* pieceBB = getTypeBB(m.from, boardCopy);
        
        u64 capture = 0ULL;

        movePiece(*pieceBB, m.from, m.to, side, boardCopy);
        int eval = evaluate(boardCopy, side, moves);
        
        if (m.from == lastTo && m.to == lastFrom) {
            eval -= 20; // discourage reversals
        }
        
        std::cerr << eval << ", ";
        if ((!bestMoveEval) || (eval > bestMoveEval)) {
            bestMoveEval = eval;
            from = m.from;
            to = m.to;
        } 
         
        undoMoveEval(*pieceBB, m.from, m.to, side, boardCopy, capture);
        lastFrom = from;
        lastTo = to;

    }
    //end

    std::string selectedMove = "from " + std::to_string(from) + " to " + std::to_string(to);


    u64* pieceBB = getTypeBB(from, board);
    if (!pieceBB) {
        return selectedMove + " (no piece at from)";
    }

    movePiece(*pieceBB, from, to, side, board); 

    board.all_white = 
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black = 
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    return selectedMove;
}




int alphaBeta(Board& board, int depth, int alpha, int beta, Color side, std::vector<Move> moves) {
    
    moves.clear();
    generateMoves(board, side, moves);
    
    if (depth == 0 /*|| position == GameOver*/) {
        logToFile("ab: depth = 0, evaluating");
        std::cerr << "ab: moves.size1 = " << moves.size() << std::endl;
        int score = evaluate(board, side, moves);
        std::cerr << "ab: moves.size2 = " << moves.size() << std::endl;         
        return score;
    };

    
    std::cerr << "ab: depth = " << depth << std::endl; 
    std::cerr << "ab: moves.size = " << moves.size() << std::endl; 

    int value = (side == WHITE) ? -1000000 : 1000000;
    for (auto& m : moves) {
        Undo undo = makeMove(m, board, side);

        if (isKingInCheck(side, board)) {
            undoMove(m, board, side, undo);
            continue; // skip illegal move
        }
        int score = alphaBeta(board, depth - 1, alpha, beta, side, std::vector<Move>());
        undoMove(m, board, side, undo);

        value = std::max(value, score);
        alpha = std::max(alpha, score);
        if (alpha >= beta) break; // pruning
    }
    return value;

}

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

Move findBestMove(Board& board, Color side, int depth) {
    std::vector<Move> moves;
   
    logToFile("generated Moves findBestMove"); 
    generateMoves(board, side, moves);
    

    Move bestMove;
    bestMove.from = 0;
    bestMove.to = 0;
    int bestScore = (side == WHITE) ? -1000000 : 1000000;

    // --- Simple move ordering: captures first ---
    std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b){
        int av = pieceValueAtSq(board, a.to);
        int bv = pieceValueAtSq(board, b.to);
        if (av != bv) return av > bv;
        if (a.from != b.from) return a.from < b.from;
        return a.to < b.to;
    });

    
    

    bool found = false;

    for (auto& m : moves) {
        Undo undo = makeMove(m, board, side);
        if (undo.to == -1) { 
            undoMove(m, board, side, undo);
            continue;
        }

        int score = alphaBeta(board, depth, -1000000, 1000000,
                              (side == WHITE) ? BLACK : WHITE, moves);
        std::string log = numToPos(undo.from) + " " +
                  numToPos(undo.to) + ": " +
                  std::to_string(score);

        logToFile(log);
        undoMove(m, board, side, undo);

        if ((!found) ||
            (side == WHITE && score > bestScore) ||
            (side == BLACK && score < bestScore)) {
            bestScore = score;
            bestMove = m;
            found = true;
        }
    }

    if (!found) {
        // No legal move found
        std::cerr << "findBestMove: no legal moves found (returning invalid move)\n";
        logToFile("findBestMove: no legal moves found (returning invalid move)");

        Move none;
        none.from = -1;
        none.to   = -1;
        return none;
    }

    // Make the selected move on the actual board
    makeMove(bestMove, board, side);

    return bestMove;
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


void parsePositionCommand(const std::string& input, Board& board, Color &side) {
    std::istringstream iss(input);
    std::string token;
    iss >> token; // "position"

    std::string type;
    iss >> type; // "startpos" or "fen"

    if (type == "startpos") {
        board = fenUnloader(startFen);
        
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
        side = (stm == "w") ? WHITE : BLACK;
    }

    // Now check for optional "moves"
    std::string movesKeyword;
    if (iss >> movesKeyword && movesKeyword == "moves") {
        std::string moveStr;
        while (iss >> moveStr) {
            Move move = parseUCIMove(moveStr, board);
            makeMove(move, board, side);
            if (side == WHITE) {
                side = BLACK;
            } else {
                side = WHITE;
            }
        }
    }
    //print_bitboard(board.all_white);
    //print_bitboard(board.all_black);
}



int main() {
    initMasks();
    Board board;
    Color side = WHITE;
    std::string input;
    

    for (int sq = 0; sq < 64; sq++) {
        RookAttackTable[sq].resize(1ULL << rookBits[sq]);
        BishopAttackTable[sq].resize(1ULL << bishopBits[sq]);
    }
    
    std::string testFen = "NNNNNNk1/RRRRRRNR/PPPPPRPR/5P1P/8/8/8/8 w KQkq - 0 1";

    
    
    std::vector<Move> moves;
    srand(time(0)); // seed random

    int moveNr = 1;
        
    if (!(board.king_white)) {
        canCastleKingside_white = false;
        canCastleQueenside_white = false;
    } 
    if (!(board.king_black)) {
        canCastleKingside_black = false;
        canCastleQueenside_black = false;
    } 

   
    initRookAttacks();
    initBishopAttacks();

    while (std::getline(std::cin, input)) {
        if (input == "uci") {
            logToFile("GUI: uci");
            std::cout << "id name Main\n";
            std::cout << "id author Me\n";
            std::cout << "uciok\n";
            logToFile("engine: id name Main");
            logToFile("engine: id author Me");
            logToFile("engine: uciok");
        }
        else if (input == "isready") {
            logToFile("GUI: isready");
            std::cout << "readyok\n";
            logToFile("engine: readyok");
        } 
        else if (input.rfind("position", 0) == 0) {
            logToFile("GUI: " + input);
            parsePositionCommand(input, board, side);
            logToFile(printBoardAsLetters(board, true));
        }
        else if (input.rfind("go", 0) == 0) {
            logToFile("GUI: " + input);

            Move best = findBestMove(board, side, 3);

             /* std::string moveStr = evalMove(moves, side, board);
            std::cout << "Evaluted move: " << moveStr << "\n";
            print_bitboard(board.all_white);
            */
            moves.clear();
            int eval = evaluate(board, side, moves);

            //printBoardAsLetters(board, false);

            int sq = 27; // d4
            u64 occ = 0ULL;

            logToFile("Move number " + moveNr);

            logToFile("Generated moves: " + moves.size());

            logToFile("Evalution: " + eval);


            if (eval == -1000000) {
                logToFile("White king is checkmated!");
            } else if (eval == 1000000) {
                logToFile("Black king is checkmated!");
            } else if (isKingInCheck(WHITE, board) || isKingInCheck(BLACK, board)) {
                if (isKingInCheck(WHITE, board)) {
                    logToFile("White king is in check!");
                }
                if (isKingInCheck(BLACK, board)) {
                    logToFile("Black king is in check!");
                }
            } else if (eval == 0.5) {
                logToFile("Stalemate");
            }
            std::cout << "bestmove " << numToPos(best.from) + numToPos(best.to) << std::endl;
            logToFile("engine: bestmove " + numToPos(best.from) + numToPos(best.to));

            moveNr += 1;
        } else if (input == "quit") {
            logToFile("GUI: quit");
            break;
        } else if (input != "quit") {
            logToFile(input);
        }
    }

    

    // for (int t = 0; t < 500; t++) {
        
        
    //     std::string w;
    //     std::cin >> w;
    //     if (side == WHITE) {
    //         side = BLACK;
    //     } else {
    //         side = WHITE;
    //     }
        
    // }
    return 0;
}

// g++ main.cpp evaluation.cpp moveGeneration.cpp search.cpp -o bot