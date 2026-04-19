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
#include "moveGen.h"
#include "misc.h"
#include "attack.h"

using u64 = uint64_t;


Undo make_null_move(Board& board) {
    Undo u;

    u.enPassantSquare = board.enPassantSquare;
    u.castlingRights = board.castlingRights;
    u.hash = board.hash;
    u.side = board.sideToMove;

    u.nullMove = true;

    // remove EP from hash
    if (board.enPassantSquare != -1)
        board.hash ^= zobristEnPassant[board.enPassantSquare % 8];

    board.enPassantSquare = -1;

    board.hash ^= zobristSide;
    board.sideToMove = (board.sideToMove == WHITE) ? BLACK : WHITE;

    return u;
}

void undo_null_move(Board& board, Undo u) {

    board.enPassantSquare = u.enPassantSquare;
    board.castlingRights = u.castlingRights;
    board.hash = u.hash;
    board.sideToMove = u.side;



    
}


