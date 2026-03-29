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


void undoMove(Move m, Board& board, Color side, Undo u) {
    Color opp = (side == WHITE) ? BLACK : WHITE;

    u64 fromMask = 1ULL << m.from;
    u64 toMask   = 1ULL << m.to;


    // === 1. MOVE BACK ===
    if (m.promotion != NONE) {
        // remove promoted piece
        u64* promoBB = getBitboard(board, m.promotion, side);
        *promoBB &= ~toMask;

        // restore pawn
        u64* pawnBB = getBitboard(board, PAWN, side);
        *pawnBB |= fromMask;
    } else {
        u64* movedBB = getBitboard(board, u.movedPiece, side);
        *movedBB &= ~toMask;
        *movedBB |= fromMask;
    }

    // occupancy update
    if (side == WHITE) {
        board.all_white &= ~toMask;
        board.all_white |= fromMask;
    } else {
        board.all_black &= ~toMask;
        board.all_black |= fromMask;
    }

    // === 2. RESTORE CAPTURE ===
    
    if (u.wasEnPassant) {
        int capSq = u.capturedSquare;
        u64* capBB = getBitboard(board, PAWN, opp);
        *capBB |= (1ULL << capSq);
        if (opp == WHITE) board.all_white |= (1ULL << capSq);
        else board.all_black |= (1ULL << capSq);
    } else if (u.capturedPiece != NONE) {
        if (u.capturedSquare < 0 || u.capturedSquare >= 64) {
            std::cout << "BAD capturedSquare\n";
            exit(1);
        }
        u64* capBB = getBitboard(board, u.capturedPiece, opp);
        *capBB |= (1ULL << u.capturedSquare);

        if (opp == WHITE)
            board.all_white |= (1ULL << u.capturedSquare);
        else
            board.all_black |= (1ULL << u.capturedSquare);
    } 

    // === 3. UNDO CASTLING ===
    if (m.from2 != -1) {
        u64* rookBB = getBitboard(board, ROOK, side);
        *rookBB &= ~(1ULL << m.to2);
        *rookBB |= (1ULL << m.from2);

        if (side == WHITE) {
            board.all_white &= ~(1ULL << m.to2);
            board.all_white |= (1ULL << m.from2);
        } else {
            board.all_black &= ~(1ULL << m.to2);
            board.all_black |= (1ULL << m.from2);
        }
    }

    // === 4. RESTORE STATE (ALWAYS) ===
    board.enPassantSquare = u.prevEnPassant;
    board.castlingRights  = u.prevCastlingRights;
    

    board.hash = u.prevHash;

    u64 all =
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white |
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    if (all != (board.all_white | board.all_black)) {
        std::cout << "OCCUPANCY BROKEN\n";
        exit(1);
    }
    if (getPieceType(board, m.from) == NONE) {
        std::cout << "UNDO FAILED: piece missing\n";
        exit(1);
    }

}

Undo makeMove(Move m, Board& board, Color side) {
    Undo u;

    u.from = m.from;
    u.to = m.to;
    u.from2 = m.from2;
    u.to2 = m.to2;

    u.wasEnPassant = false;

    u.prevEnPassant = board.enPassantSquare;
    u.prevCastlingRights = board.castlingRights;

    u.movedPiece = getPieceType(board, m.from);
    u.capturedPiece = NONE;
    u.capturedSquare = -1;

    u.prevHash = board.hash;
    


    u64 fromMask = 1ULL << m.from;
    u64 toMask   = 1ULL << m.to;

    Color opp = (side == WHITE) ? BLACK : WHITE;
    if (u.movedPiece == NONE) {
        std::cout << "u.movedPiece: " << u.movedPiece << std::endl;
    }
    u64* fromBB = getBitboard(board, u.movedPiece, side);

    // === 1. REMOVE FROM SQUARE ===
    *fromBB &= ~fromMask;

    if (side == WHITE)
        board.all_white &= ~fromMask;
    else
        board.all_black &= ~fromMask;

    // === 2. EN PASSANT CAPTURE ===
    if (u.movedPiece == PAWN && m.to == board.enPassantSquare) {
        int capSq = (side == WHITE) ? m.to - 8 : m.to + 8;

        u.wasEnPassant = true;
        u.capturedPiece = PAWN;
        u.capturedSquare = capSq;

        u64 capMask = 1ULL << capSq;
        u64* capBB = getBitboard(board, PAWN, opp);
        *capBB &= ~capMask;

        if (opp == WHITE)
            board.all_white &= ~capMask;
        else
            board.all_black &= ~capMask;
    }
    else {
        // === NORMAL CAPTURE ===

        if (m.captured != NONE) {
            u.capturedPiece = m.captured;
            u.capturedSquare = m.to;

            if (m.captured == NONE) {
                std::cerr << "captured: " << m.captured << std::endl;
            }
            u64* capBB = getBitboard(board, m.captured, opp);
            *capBB &= ~toMask;

            if (opp == WHITE)
                board.all_white &= ~toMask;
            else
                board.all_black &= ~toMask;
        }
        if (u.capturedPiece == ROOK) {
            if (opp == WHITE) {
                if (u.capturedSquare == 0) board.castlingRights &= ~WQ;
                if (u.capturedSquare == 7) board.castlingRights &= ~WK;
            } else {
                if (u.capturedSquare == 56) board.castlingRights &= ~BQ;
                if (u.capturedSquare == 63) board.castlingRights &= ~BK;
            }
        }
    }

    

    // === 4. MOVE PIECE / PROMOTION ===
    if (m.promotion != NONE) {
        if (!m.promotion) {
            std::cerr << "m.promotion: " << m.promotion << std::endl;
        }
        u64* promoBB = getBitboard(board, m.promotion, side);
        *promoBB |= toMask;
    } else {
        *fromBB |= toMask;
    }

    // === 5. ADD TO OCCUPANCY ===
    if (side == WHITE)
        board.all_white |= toMask;
    else
        board.all_black |= toMask;

    // === 6. CASTLING ===
    if (m.from2 != -1) {
        u64 rookFrom = 1ULL << m.from2;
        u64 rookTo   = 1ULL << m.to2;

        u64* rookBB = getBitboard(board, ROOK, side);

        *rookBB &= ~rookFrom;
        *rookBB |= rookTo;

        if (side == WHITE) {
            board.all_white &= ~rookFrom;
            board.all_white |= rookTo;
    
        } else {
            board.all_black &= ~rookFrom;
            board.all_black |= rookTo;
         
        }
        // Clear castling rights for this side
        if (side == WHITE) {
            board.castlingRights &= ~(WK | WQ);
        } else {
            board.castlingRights &= ~(BK | BQ);
        }
    }

    // === 7. SET EN PASSANT SQUARE ===
    if (u.movedPiece == PAWN && abs(m.to - m.from) == 16) {
        board.enPassantSquare = (side == WHITE) ? m.from + 8 : m.from - 8;
    } else {
        board.enPassantSquare = -1;
    }

    if (!perfTest) board.hash = computeHash(board, (side == WHITE) ? BLACK : WHITE);

    if (u.movedPiece == NONE) {
        std::cout << "ERROR: moving empty square\n";
        std::cout << "from: " << m.from << "\n";
        exit(1);
    }

    if (u.movedPiece == KING) {
        if (side == WHITE)
            board.castlingRights &= ~(WK | WQ);
        else
            board.castlingRights &= ~(BK | BQ);
    }
    if (u.movedPiece == ROOK) {
        if (side == WHITE) {
            if (m.from == 0) board.castlingRights &= ~WQ; // a1 rook
            if (m.from == 7) board.castlingRights &= ~WK; // h1 rook
        } else {
            if (m.from == 56) board.castlingRights &= ~BQ; // a8
            if (m.from == 63) board.castlingRights &= ~BK; // h8
        }
    }
    return u;
}