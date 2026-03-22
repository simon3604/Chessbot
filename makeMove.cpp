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
    if (u.capturedPiece != NONE) {
        if (u.wasEnPassant) {
            u64* capBB = getBitboard(board, PAWN, opp);
            *capBB |= (1ULL << u.capturedSquare);

            if (opp == WHITE)
                board.all_white |= (1ULL << u.capturedSquare);
            else
                board.all_black |= (1ULL << u.capturedSquare);
        } else {
            u64* capBB = getBitboard(board, u.capturedPiece, opp);
            *capBB |= toMask;

            if (opp == WHITE)
                board.all_white |= toMask;
            else
                board.all_black |= toMask;
        }
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
    
    board.hash = computeHash(board, side);

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


    u64 fromMask = 1ULL << m.from;
    u64 toMask   = 1ULL << m.to;

    Color opp = (side == WHITE) ? BLACK : WHITE;

    u64* fromBB = getBitboard(board, u.movedPiece, side);

    // === 1. REMOVE FROM SQUARE ===
    *fromBB &= ~fromMask;

    if (side == WHITE)
        board.all_white &= ~fromMask;
    else
        board.all_black &= ~fromMask;

    // === 2. CAPTURE ===
    Piece captured = getPieceType(board, m.to);

    if (captured != NONE) {
        u.capturedPiece = captured;

        u64* capBB = getBitboard(board, captured, opp);
        *capBB &= ~toMask;

        if (opp == WHITE)
            board.all_white &= ~toMask;
        else
            board.all_black &= ~toMask;
    }

    // === 3. EN PASSANT CAPTURE ===
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

    // === 4. MOVE PIECE / PROMOTION ===
    if (m.promotion != NONE) {
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
    }

    // === 7. SET EN PASSANT SQUARE ===
    if (u.movedPiece == PAWN && abs(m.to - m.from) == 16) {
        board.enPassantSquare = (side == WHITE) ? m.from + 8 : m.from - 8;
    } else {
        board.enPassantSquare = -1;
    }

    board.hash = computeHash(board, (side == WHITE) ? BLACK : WHITE);
    return u;
}