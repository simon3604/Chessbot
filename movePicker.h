#pragma once
#include "constants.h"
#include "moveGen.h"

using u32 = uint32_t;

extern int history[2][64][64];
extern u32 killerMoves[64][2];
extern u32 moveStack[64][256];
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

    MovePicker(Board& b, u32 tt, int ply);

    u32 nextMove();

    

private:
    Board& board;
    u32 ttMove;      
    int ply;            
    int index;          // current move 
    int moveCount;
    int killerIndex;
    Stage stage;     
    u32 moves[MAX_MOVES];
    int scores[MAX_MOVES];
    u32 captures[MAX_MOVES];
    u32 quiets[MAX_MOVES];

    int captureCount = 0;
    int quietCount = 0;

    int captureStart;
    int captureEnd;
    int quietStart;
    int quietEnd;

    void scoreMoves(int start, int end);
    
    u32 pickBest();
};