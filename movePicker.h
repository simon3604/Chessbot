#pragma once
#include "constants.h"
#include "moveGen.h"

extern int history[2][64][64];
extern Move killerMoves[64][2];
extern Move moveStack[64][256];
extern int moveScores[64][256];

enum Stage {
    TT_STAGE,
    CAPTURE_STAGE,
    KILLER_STAGE,
    QUIET_STAGE,
    DONE_STAGE
};

class MovePicker {
public:
    static const int MAX_MOVES = 256;

    MovePicker(Board& b, Move tt, int ply);

    Move nextMove();

    

private:
    Board& board;
    Move ttMove;      
    int ply;            
    int index;          // current move 
    int moveCount;
    int killerIndex;
    Stage stage;     
    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    Move captures[MAX_MOVES];
    Move quiets[MAX_MOVES];

    int captureCount = 0;
    int quietCount = 0;

    int captureStart;
    int captureEnd;
    int quietStart;
    int quietEnd;

    void scoreMoves(int start, int end);
    
    Move pickBest();
};