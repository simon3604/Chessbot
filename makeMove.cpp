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



// Undo a move
void undoMove(Move m, Board& board, Color side, Undo undo) {
    // If the move was a promotion: remove promoted piece and restore pawn
    if (m.promotion != NONE) {
        if (undo.movedPiece) {
            // remove the promoted piece from its destination square
            *undo.movedPiece &= ~(1ULL << m.to);
        }
        // restore pawn at the original square
        u64* pawnBB = (side == WHITE) ? &board.pawns_white : &board.pawns_black;
        
        *pawnBB &= ~(1ULL << m.to);
        *pawnBB |= (1ULL << m.from);
    } else {
        // Normal move undo: move the bit back from 'to' -> 'from'
        if (undo.movedPiece) {
            *undo.movedPiece &= ~(1ULL << m.to);
            *undo.movedPiece |=  (1ULL << m.from);
        }
    }

    if (undo.capturedPiece) {
        *undo.capturedPiece |= undo.capturedMask;
    }

    // // Undo rook move from castling
    // if (undo.movedRook) {
    //     *undo.movedRook &= ~(1ULL << m.to2);
    //     *undo.movedRook |=  (1ULL << m.from2);
    // }

    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white  | board.queens_white  | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black  | board.queens_black  | board.king_black;
    //std::cout << "undoMove";             
             
}


// Make a move on the board and return undo info
Undo makeMove(Move m, Board& board, Color side) {
    Undo undo;
    undo.from = m.from;
    undo.to = m.to;
    undo.from2 = m.from2;
    undo.to2 = m.to2;
    undo.movedPiece = getTypeBB(m.from, board);
    undo.capturedPiece = nullptr;
    undo.capturedMask = 1ULL << m.to;

    u64 oppSideBB = (side == WHITE) ? board.all_black : board.all_white;



    if (!undo.movedPiece) {return undo;} // safety check


    // capture
    u64 targetSq = 1ULL << m.to;

    

    if (targetSq & oppSideBB) {
        undo.capturedPiece = getTypeBB(m.to, board);
        if (undo.capturedPiece) {
            undo.capturedMask = targetSq;
            *undo.capturedPiece &= ~targetSq;
        }
    }


    // Promotion for non-pawn (safety)
    if ((m.promotion != NONE) && !((1ULL << m.from) & ((side == WHITE) ? board.pawns_white : board.pawns_black))) {
    std::cerr << "⚠️ ERROR: Promotion set on a non-pawn move! from=" << m.from
              << " to=" << m.to << "\n";
    }
    

    //Check if the piece moved is a pawn
    u64 fromSq = 1ULL << m.from;
    bool isPawn = (side == WHITE)
        ? (fromSq & board.pawns_white)
        : (fromSq & board.pawns_black);

  


    // === Handle promotion ===
    if (m.promotion != NONE) {
        // Remove pawn from its bitboard
        *undo.movedPiece &= ~(1ULL << m.from);

        // Add promoted piece to the right bitboard
        u64* promoBB = nullptr;
        switch (m.promotion) {
            case QUEEN:
                promoBB = (side == WHITE) ? &board.queens_white : &board.queens_black;
                break;
            case ROOK:
                promoBB = (side == WHITE) ? &board.rooks_white : &board.rooks_black;
                break;
            case BISHOP:
                promoBB = (side == WHITE) ? &board.bishops_white : &board.bishops_black;
                break;
            case KNIGHT:
                promoBB = (side == WHITE) ? &board.knights_white : &board.knights_black;
                break;
            default:
                promoBB = undo.movedPiece; // just in case
                break;
        }

        *promoBB |= (1ULL << m.to);
        undo.movedPiece = promoBB; // track promoted piece for undo
    }
    else {
        // === Normal move ===
        *undo.movedPiece &= ~(1ULL << m.from);
        *undo.movedPiece |=  (1ULL << m.to);
    }

    // // === Handle castling rook ===
    // if (m.from2 != -1 && m.to2 != -1) {
    //     undo.movedRook = getTypeBB(m.from2, board);
    //     if (undo.movedRook) {
    //         *undo.movedRook &= ~(1ULL << m.from2);
    //         *undo.movedRook |=  (1ULL << m.to2);
    //     }
    // }

    

    // === En Passant === 
    if (isPawn && abs(undo.to - undo.from) == 16) {
        board.enPassantSquare = (side == WHITE) ? undo.from + 8 : undo.from - 8;
    } else {
        board.enPassantSquare = -1;
    }


    // === Update bitboards ===
    board.all_white =
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black =
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    
    return undo;
}

