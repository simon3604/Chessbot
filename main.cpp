#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>
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

    seedRNG(0x9e3779b97f4a7c15ULL);
    initZobrist();

    //init pst
    const int* tables[6] = {
        PST_PAWN,
        PST_KNIGHT,
        PST_BISHOP,
        PST_ROOK,
        PST_QUEEN,
        PST_KING
    };

    for (int p = 0; p < 6; p++) {
        for (int sq = 0; sq < 64; sq++) {

            // white pieces
            pst[p][sq] = tables[p][sq];

            // black pieces (mirrored + negated)
            pst[p + 6][sq] = -tables[p][sq ^ 56];
        }
    }

    Move moveStack[MAX_PLY][MAX_MOVES];

    
    Board board{};
    Color side = WHITE;
    std::string input;
    

    for (int sq = 0; sq < 64; sq++) {
        RookAttackTable[sq].resize(1ULL << rookBits[sq]);
        BishopAttackTable[sq].resize(1ULL << bishopBits[sq]);
    }
    
    
    
    
    srand(time(0)); // seed random

    

    std::thread searchThread; 

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
            std::istringstream iss(input); 
            std::string token; iss >> token; 
            GoParams go {}; 
            
            while (iss >> token) { 
                if (token == "wtime") 
                    iss >> go.wtime; 
                else if (token == "btime") 
                    iss >> go.btime; 
                else if (token == "winc") 
                    iss >> go.winc; 
                else if (token == "binc") 
                    iss >> go.binc; 
                else if (token == "movestogo") 
                    iss >> go.movestogo; 
                else if (token == "depth") 
                    iss >> go.depth; 
                else if (token == "movetime") 
                    iss >> go.movetime; 
                else if (token == "infinite") 
                go.infinite = true; 
            } 
            if ((!go.wtime )|| (!go.btime)) { go.infinite = true; } 
            
            if (searchThread.joinable())
                searchThread.join();

            searchThread = std::thread([&board, go]() {
                Move best = search(board, go);
                
                    std::string move = numToPos(best.from) + numToPos(best.to);
                    if (best.promotion != NONE)
                        std::cout << "bestmove " << move << getPieceLetter(best.promotion) << std::endl;
                    else
                        std::cout << "bestmove " << move << std::endl;
                    logToFile("engine: bestmove " + move);
                
            });
       
        } else if (input == "quit") {
            logToFile("GUI: quit");
            stopSearch = true;          // stop any running search
            if (searchThread.joinable())
                searchThread.join();    // wait for it to finish
            break;
        }
        else if (input == "stop") {
            stopSearch = true;          // signal the search thread to stop
            
            logToFile(input);
            if (searchThread.joinable())
                searchThread.join();    // wait for it to finish
        } 
        else if (input == "evaluate") {
            logToFile(input);
            std::cout << std::to_string(evaluate(board, side)) << std::endl;
            
        }
        else if (input.rfind("perft", 0) == 0) {
            logToFile(input);
            perfTest = true;
            std::istringstream iss(input);
            std::string token;
            int depth;

            iss >> token >> depth; 
            u64 captures = 0ULL;
            u64 promotions = 0ULL; 
            u64 enPassants = 0ULL; 
            u64 castles = 0ULL; 
            u64 checks = 0ULL; 
            u64 checkmates = 0ULL; 
            
            auto start = std::chrono::high_resolution_clock::now();


            std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now(); 


            u64 nodeCount = perft(board, 0, depth, captures, promotions, castles, enPassants, checks, checkmates);
            std::cout << "Perft in progress" << std::endl;

            auto end = std::chrono::high_resolution_clock::now();

            double seconds = std::chrono::duration<double>(end - start).count();

            std::cout << "==== Perft results ====" << std::endl;
            std::cout << "Nodes: " << nodeCount << std::endl;
            std::cout << "Seconds: " << seconds << std::endl;
            std::cout << "NPS: " << nodeCount / seconds << std::endl;
            std::cout << "Captures: " << captures << std::endl;
            std::cout << "E.p.: " << enPassants << std::endl;
            std::cout << "Castles: " << castles << std::endl;
            std::cout << "Promotions: " << promotions << std::endl;
            std::cout << "Checks: " << checks << std::endl;
            std::cout << "Checkmates: " << checkmates << std::endl;

            perfTest = false;
        }
        else if (input.rfind("benchmark", 0) == 0) {
            logToFile(input);
            std::istringstream iss(input);
            std::string token;
            int depth;

            iss >> token >> depth;  
            benchmarkSearch(board, depth, board.sideToMove);          
        }
        else if (input != "quit") {
            logToFile(input);
        }
    }

    

    // join any running search thread before exit
    if (searchThread.joinable())
        searchThread.join();
    return 0;
}

// g++ main.cpp evaluation.cpp moveGeneration.cpp search.cpp -o bot