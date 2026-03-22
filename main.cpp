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
    initZobrist();
    
    Board board{};
    Color side = WHITE;
    std::string input;
    bool stopSearch;
    

    for (int sq = 0; sq < 64; sq++) {
        RookAttackTable[sq].resize(1ULL << rookBits[sq]);
        BishopAttackTable[sq].resize(1ULL << bishopBits[sq]);
    }
    
    
    
    
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

            Move best = findBestMove(board, sideToMove, 7);
            
            sideToMove = (sideToMove == WHITE) ? BLACK :  WHITE;




            
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
        else if (input == "evaluate") {
            logToFile(input);
            std::cout << std::to_string(evaluate(board, sideToMove)) << std::endl;
            
        }
        else if (input == "perft") {
            logToFile(input);
            std::cout << perft(board, 6, sideToMove) << std::endl;
            
        }
        else if (input.rfind("benchmark", 0) == 0) {
            logToFile(input);
            std::istringstream iss(input);
            std::string token;
            int depth;

            iss >> token >> depth;  
            benchmarkSearch(board, depth, sideToMove);          
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