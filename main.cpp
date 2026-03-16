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
#include "moveGen.h"
#include "search.h"
#include "constants.h"
#include "misc.h"


using u64 = uint64_t;





int main() {
    initMasks();
    initAttacks();
    
    Board board{};
    Color side = WHITE;
    std::string input;
    bool stopSearch;
    

    for (int sq = 0; sq < 64; sq++) {
        RookAttackTable[sq].resize(1ULL << rookBits[sq]);
        BishopAttackTable[sq].resize(1ULL << bishopBits[sq]);
    }
    
    
    
    
    std::vector<Move> moves;
    srand(time(0)); // seed random

        
    if (!(board.king_white)) {
        canCastleKingside_white = false;
        canCastleQueenside_white = false;
    } 
    if (!(board.king_black)) {
        canCastleKingside_black = false;
        canCastleQueenside_black = false;
    } 

   
    

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

            stopSearch = false;

            Move best = findBestMove(board, sideToMove, 1);
            
            sideToMove = (sideToMove == WHITE) ? BLACK :  WHITE;

            moves.clear();
            int eval = evaluate(board, side, moves);


            
        
            
            generateLegalMoves(board, side, moves);
            


            logToFile("Generated moves: " + moves.size());

            logToFile("Evalution: " + eval);

            moves.clear();


            if (eval == -1000000) {
                logToFile("White king is checkmated!");
            } 
            else if (eval == 1000000) {
                logToFile("Black king is checkmated!");
            } 
            else if (eval == 0 && moves.empty()) {
                logToFile("Stalemate");
            }
            std::string move = numToPos(best.from) + numToPos(best.to);

            std::cout << "bestmove " << move << std::endl;
            logToFile("engine: bestmove " + move);
        } else if (input == "quit") {
            logToFile("GUI: quit");
            break;
        } else if (input == "stop") {
            logToFile("GUI: stop");
            stopSearch = true;
           
        } 
        else if (input == "test") {
            logToFile(input);
            
        }
        else if (input != "quit") {
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