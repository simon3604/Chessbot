#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <bits/stdc++.h>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include "constants.h"
#include "moveGen.h"
#include "misc.h"
#include "attack.h"

using u64 = uint64_t;
using u32 = uint32_t;

inline void undoMove(const u32 &m, Board &board, const Undo &u, std::string debug)
{

    // 1. restore  state
    board.castlingRights = u.castlingRights;
    board.enPassantSquare = u.enPassantSquare;
    board.halfmoveClock = u.halfmoveClock;
    board.eval = u.eval;
    board.sideToMove = u.side;
    board.hash = u.hash;

    int from = fromSquare(m);
    int to = toSquare(m);

    u64 fromBB = 1ULL << from;
    u64 toBB = 1ULL << to;
    u64 fromTo = fromBB | toBB;

    Piece p = (moveFlags(m) & PROMOTION) ? ((board.sideToMove == WHITE) ? WP : BP) : getPieceType(board, to);
    Color c = pieceColor(p, "undoMove");

    // 1. CASTLING

    if (moveFlags(m) & CASTLING)
    {
        u64 rookMask = (1ULL << getRookFromSquare(m, c)) | (1ULL << getRookToSquare(m, c));
        Piece rook = (c == WHITE) ? WR : BR;

        board.pieces[rook] ^= rookMask;

        if (c == WHITE)
            board.all_white ^= rookMask;
        else
            board.all_black ^= rookMask;

        board.all ^= rookMask;
    }

    // 2. PROMOTION

    if (moveFlags(m) & PROMOTION)
    {

        if (p == NONE)
        {
            std::cout << "undoMove: Promotion piece type is NONE. Debug info: " << debug << std::endl;
            printBoardAsLetters(board, false);
            printMove(board, m);
        }

        board.pieces[promotionPiece(m)] ^= toBB;

        board.pieces[p] ^= fromBB;
    }
    else

    // 3. NORMAL MOVE
    {
        if (p == NONE)
        {
            std::cout << "undoMove: Piece type is NONE. Debug info: " << debug << std::endl;
            printBoardAsLetters(board, false);
            printMove(board, m);
        }

        board.pieces[p] ^= fromTo;
    }

    if (c == WHITE)
        board.all_white ^= fromTo;
    else
        board.all_black ^= fromTo;

    board.all ^= fromTo;

    if (moveFlags(m) & CAPTURE)
    {

        // 4. EN PASSANT
        if (moveFlags(m) & ENPASSANT)
        {
            int capSq = (c == WHITE) ? to - 8 : to + 8;
            u64 capBB = 1ULL << capSq;

            board.pieces[u.capturedPiece] ^= capBB;

            if (c == WHITE)
                board.all_black ^= capBB;
            else
                board.all_white ^= capBB;

            board.all ^= capBB;
        }
        else

        // 5. NORMAL CAPTURE
        {
            board.pieces[u.capturedPiece] ^= toBB;

            if (c == WHITE)
                board.all_black ^= toBB;
            else
                board.all_white ^= toBB;

            board.all ^= toBB;
        }
    }

    const u64 recomputed =
        board.pieces[WP] | board.pieces[WN] | board.pieces[WB] |
        board.pieces[WR] | board.pieces[WQ] | board.pieces[WK] |
        board.pieces[BP] | board.pieces[BN] | board.pieces[BB] |
        board.pieces[BR] | board.pieces[BQ] | board.pieces[BK];

    if (recomputed != board.all)
    {
        std::cout << "Undo: board.all BROKEN\n";
        printMove(board, m);
        std::cout << "Debug info: " << debug << std::endl;
        std::cout << "Recomputed: " << std::endl;
        print_bitboard(recomputed);
        printBoardAsLetters(board, false);
        printBoard(board);
        exit(1);
    }
}

inline Undo makeMove(u32 m, Board &board, std::string calledFrom)
{    
    Undo u;

    Piece target = getPieceType(board, toSquare(m));

    if (target != NONE && moveFlags(m) & QUIET)
    {
        std::cout << "❌ ILLEGAL NON-CAPTURE ON OCCUPIED SQUARE\n";
        std::cout << "Called from: " << calledFrom << std::endl;
        printMove(board, m);
        exit(1);
    }

    Color side = pieceColor(getPieceType(board, fromSquare(m)), "makeMove");

    if (side != board.sideToMove)
    {
        std::cout << "❌ ILLEGAL MOVE: WRONG SIDE TO MOVE\n";
        std::cout << "Called from: " << calledFrom << std::endl;
        printMove(board, m);
    }

    else if (board.pieces[target] & ((board.sideToMove == WHITE) ? board.all_white : board.all_black))
    {
        std::cout << "❌ ILLEGAL MOVE: CAPTURE OWN PIECE\n";
        std::cout << "Called from: " << calledFrom << std::endl;
        printMove(board, m);
        std::cout << "Side to move: " << ((board.sideToMove == WHITE) ? "WHITE" : "BLACK") << std::endl;
        std::cout << "Target piece: " << numToPiece(target) << std::endl;
        printBoardAsLetters(board, false);
    }

    // 1. Save state
    u.castlingRights = board.castlingRights;
    u.enPassantSquare = board.enPassantSquare;
    u.halfmoveClock = board.halfmoveClock;
    u.hash = board.hash;
    u.eval = board.eval;
    u.side = board.sideToMove;

    int score = 0;
    int from = fromSquare(m);
    int to = toSquare(m);

    u64 fromBB = 1ULL << from;
    u64 toBB = 1ULL << to;
    u64 fromTo = fromBB | toBB;

    Piece p = getPieceType(board, from);
    Color c = board.sideToMove;

    if (p == NONE)
    {
        std::cout << "ILLEGAL MOVE (empty square), called from: " << calledFrom << "\n";
        printBoardAsLetters(board, false);
        printMove(board, m);
        exit(1);
    }

    u64 &pieceBB = board.pieces[p];

    // 2. Remove old ep
    if (board.enPassantSquare != -1)
    {
        board.hash ^= zobristEnPassant[board.enPassantSquare % 8];
    }
    board.enPassantSquare = -1;

    // 3. CAPTURE
    if ((moveFlags(m) & CAPTURE) && !(moveFlags(m) & ENPASSANT))
    {
        Piece cap = getPieceType(board, to);
        u.capturedPiece = cap;

        board.pieces[cap] ^= toBB;
        board.hash ^= zobrist[cap][to];
        score -= pst[cap][to];

        if (c == WHITE)
            board.all_black ^= toBB;
        else
            board.all_white ^= toBB;

        board.all ^= toBB;
    }
    // 4. EN PASSANT
    if (moveFlags(m) & ENPASSANT)
    {
        int capSq = (c == WHITE) ? to - 8 : to + 8;
        u64 capBB = 1ULL << capSq;
        Piece cap = getPieceType(board, capSq);
        u.capturedPiece = cap;

        board.pieces[cap] ^= capBB;
        board.hash ^= zobrist[cap][capSq];

        if (c == WHITE)
            board.all_black ^= capBB;
        else
            board.all_white ^= capBB;

        board.all ^= capBB;
    }

    // 5. MOVE PIECE
    pieceBB ^= fromTo;
    board.hash ^= zobrist[p][from];
    board.hash ^= zobrist[p][to];

    if (c == WHITE)
        board.all_white ^= fromTo;
    else
        board.all_black ^= fromTo;

    board.all ^= fromTo;

    // 6. PROMOTION
    if (moveFlags(m) & PROMOTION)
    {
        // remove pawn
        board.pieces[p] ^= toBB;
        board.hash ^= zobrist[p][to];

        // add promoted piece
        board.pieces[promotionPiece(m)] ^= toBB;
        board.hash ^= zobrist[promotionPiece(m)][to];

        // update pst
        score -= pst[p][to];
        score += pst[promotionPiece(m)][to];
    }

    // 7. CASTLING

    if (moveFlags(m) & CASTLING)
    {

        int rookFrom = getRookFromSquare(m, c);
        int rookTo = getRookToSquare(m, c);
        Piece rook = (c == WHITE) ? WR : BR;

        board.pieces[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
        board.hash ^= zobrist[rook][rookFrom];
        board.hash ^= zobrist[rook][rookTo];

        u64 rookMask = (1ULL << rookFrom) | (1ULL << rookTo);

        if (c == WHITE)
            board.all_white ^= rookMask;
        else
            board.all_black ^= rookMask;

        board.all ^= rookMask;

        if (rook == WR)
        {
            if (rookFrom == 0)
                board.castlingRights &= ~CWQ;
            if (rookFrom == 7)
                board.castlingRights &= ~CWK;
        }
        else if (rook == BR)
        {

            if (rookFrom == 56)
                board.castlingRights &= ~CBQ;
            if (rookFrom == 63)
                board.castlingRights &= ~CBK;
        }
    }

    // 8. UPDATE STATE

    // Update castling rights
    if (p == WK)
    {
        board.castlingRights &= ~(CWK | CWQ);
    }
    else if (p == BK)
    {
        board.castlingRights &= ~(CBK | CBQ);
    }

    if (p == WR)
    {
        if (from == 0)
        {
            board.castlingRights &= ~CWQ;
        }
        if (from == 7)
        {
            board.castlingRights &= ~CWK;
        }
    }
    else if (p == BR)
    {
        if (from == 56)
        {
            board.castlingRights &= ~CBQ;
        }
        if (from == 63)
        {
            board.castlingRights &= ~CBK;
        }
    }


    // Update ep square
    if (p == WP || p == BP)
    {
        if (abs(to - from) == 16)
        {
            board.enPassantSquare = (from + to) / 2;
        }
    }
    if (board.enPassantSquare != -1)
        board.hash ^= zobristEnPassant[board.enPassantSquare % 8];

    // 10. HALF MOVE CLOCK
    if (p == WP || p == BP || moveFlags(m) & CAPTURE)
        board.halfmoveClock = 0;
    else
        board.halfmoveClock++;

    //  SWITCH SIDE
    board.sideToMove = (board.sideToMove == WHITE) ? BLACK : WHITE;
    board.hash ^= zobristSide;


    // Update pst
    score -= pst[p][from];
    score += pst[p][to];

    board.eval += score;


    return u;
}

Undo make_null_move(Board &board);

void undo_null_move(Board &board, Undo u);
