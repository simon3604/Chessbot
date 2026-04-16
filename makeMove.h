#pragma once

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



inline void undoMove(const Move& m, Board& board, const Undo& u) {

    // 1. restore  state
    board.castlingRights = u.castlingRights;
    board.enPassantSquare = u.enPassantSquare;
    board.halfmoveClock = u.halfmoveClock;
    board.eval = u.eval;

    // 2. switch side 
    board.sideToMove = u.side;

    int from = m.from;
    int to   = m.to;

    u64 fromBB = 1ULL << from;
    u64 toBB   = 1ULL << to;
    u64 fromTo = fromBB | toBB;

    Piece p = m.piece;
    Color c = (p <= WK) ? WHITE : BLACK;
 
    // Promotion
    if (m.flags & PROMOTION) {
        board.pieces[m.promotion] ^= toBB;

        board.pieces[p] ^= fromBB;

        if (c == WHITE)
            board.all_white ^= fromBB;
        else
            board.all_black ^= fromBB;

        board.all ^= fromBB;

        if (c == WHITE)
            board.all_white ^= toBB;
        else
            board.all_black ^= toBB;

        board.all ^= toBB;
    }
    else {
        // normal move
        board.pieces[p] ^= fromTo;

        if (c == WHITE)
            board.all_white ^= fromTo;
        else
            board.all_black ^= fromTo;

        board.all ^= fromTo;
    }

    // === CAPTURE ===
    if (m.flags & CAPTURE) {

        if (m.flags & ENPASSANT) {
            int capSq = (c == WHITE) ? to - 8 : to + 8;
            u64 capBB = 1ULL << capSq;

    

            board.pieces[m.captured] ^= capBB;

            if (c == WHITE)
                board.all_black ^= capBB;
            else
                board.all_white ^= capBB;

            board.all ^= capBB;
        }
        else {
            board.pieces[m.captured] ^= toBB;

            if (c == WHITE)
                board.all_black ^= toBB;
            else
                board.all_white ^= toBB;

            board.all ^= toBB;
        }
    }

    // === CASTLING ===
    if (m.flags & CASTLING) {
        u64 rookMask = (1ULL << m.from2) | (1ULL << m.to2);
        Piece rook = (c == WHITE) ? WR : BR;

        board.pieces[rook] ^= rookMask;

        if (c == WHITE)
            board.all_white ^= rookMask;
        else
            board.all_black ^= rookMask;

        board.all ^= rookMask;
    }

    // 3. restore hash 
    board.hash = u.hash;
}

inline Undo makeMove(Move m, Board& board) {
    Undo u;

   

    

    //1. Save state
    u.castlingRights = board.castlingRights;
    u.enPassantSquare = board.enPassantSquare;
    u.halfmoveClock = board.halfmoveClock;
    u.hash = board.hash;
    u.eval = board.eval;
    u.side = board.sideToMove;
    
    
    int score = 0;
    int from = m.from;
    int to   = m.to;
    int from2 = m.from2;
    int to2 = m.to2;

    u64 fromBB = 1ULL << from;
    u64 toBB   = 1ULL << to;
    u64 fromTo = fromBB | toBB;

    Piece p = m.piece;
    Color c = (p <= WK) ? WHITE : BLACK;
     Piece real = getPieceType(board, from);
    if (real == NONE) {
        std::cout << "ILLEGAL MOVE (empty square)\n";
        exit(1);
    }

    if (real != m.piece && m.piece != NONE) {
        std::cout << "PIECE MISMATCH\n";
        exit(1);
    }

    if (!(board.pieces[m.piece] & (1ULL << m.from))) {
        std::cout << "WRONG PIECE!\n";
        std::cout << "m.piece: " << m.piece << std::endl;
        std::cout << "m.from: " << getPieceType(board, m.from) << std::endl;
        printBoardAsLetters(board, false);
        print_bitboard(board.pieces[m.piece]);
        print_bitboard(1ULL << m.from);
        exit(1);
    }

    u64& pieceBB = board.pieces[m.piece];

    //2. Remove old ep
    if (board.enPassantSquare != -1) {
        board.hash ^= zobristEnPassant[board.enPassantSquare % 8];
        
    }
    board.enPassantSquare = -1;
    
    // 3. MOVE PIECE 
    pieceBB ^= fromTo;
    board.hash ^= zobrist[p][from];
    board.hash ^= zobrist[p][to];


    if (c == WHITE)
        board.all_white ^= fromTo;
    else
        board.all_black ^= fromTo;

    board.all ^= fromTo;

    // === CAPTURE ===
    if ((m.flags & CAPTURE) && !(m.flags & ENPASSANT)) {
        Piece cap = m.captured;

        board.pieces[cap] ^= toBB;
        board.hash ^= zobrist[cap][to];
        score -= pst[cap][to];

        if (c == WHITE)
            board.all_black ^= toBB;
        else
            board.all_white ^= toBB;

        board.all ^= toBB;
    }
    // === EN PASSANT ===
    if (m.flags & ENPASSANT) {
        int capSq = (c == WHITE) ? to - 8 : to + 8;
        u64 capBB = 1ULL << capSq;

       

        board.pieces[m.captured] ^= capBB;
        board.hash ^= zobrist[m.captured][capSq];

        if (c == WHITE)
            board.all_black ^= capBB;
        else
            board.all_white ^= capBB;

        board.all ^= capBB;
    }
    // === PROMOTION ===
    if (m.flags & PROMOTION) {
        // remove pawn
        board.pieces[p] ^= toBB;
        board.hash ^= zobrist[p][to];

        // add promoted piece
        board.pieces[m.promotion] ^= toBB;
        board.hash ^= zobrist[m.promotion][to];

        //update pst
        score -= pst[p][to];
        score += pst[m.promotion][to];

    }

    

    // === CASTLING ===
    if (m.flags & CASTLING) {

        u64 rookMask = (1ULL << from2) | (1ULL << to2);

        Piece rook = (c == WHITE) ? WR : BR;

        board.pieces[rook] ^= rookMask;
        board.hash ^= zobrist[rook][m.from2];
        board.hash ^= zobrist[rook][m.to2];

        if (c == WHITE)
            board.all_white ^= rookMask;
        else
            board.all_black ^= rookMask;

        board.all ^= rookMask;
    }

    
    
    // === UPDATE CASTLINGRIGHTS ===



    if (m.captured == WR) {
        if (to == 0) board.castlingRights &= ~CWQ;
        if (to == 7) board.castlingRights &= ~CWK;

        
    } else if (m.captured == BR) {

        if (to == 56) board.castlingRights &= ~CBQ;
        if (to == 63) board.castlingRights &= ~CBK;
    }


    if (p == WK) {
            board.castlingRights &= ~(CWK | CWQ);
    
    }
    else if (p == BK) {
            board.castlingRights &= ~(CBK | CBQ);
    
    }

   
    if (p == WR) {
        if (from == 0) {
            board.castlingRights &= ~CWQ;
    
        } 
        if (from == 7) {
            board.castlingRights &= ~CWK;
        
        }
    } else if (p == BR) {
        if (from == 56) {
            board.castlingRights &= ~CBQ;
            
        }
        if (from == 63) {
            board.castlingRights &= ~CBK;
        
        }
    }


    if (board.castlingRights != u.castlingRights) {
        board.hash ^= zobristCastling[u.castlingRights];
        board.hash ^= zobristCastling[board.castlingRights];
    }


    //=== NEW EP ===
    if (p == WP || p == BP) {
        if (abs(to - from) == 16) {
            board.enPassantSquare = (from + to) / 2;
        }
        }
        if (board.enPassantSquare != -1)
            board.hash ^= zobristEnPassant[board.enPassantSquare % 8];



    // === HALF MOVE CLOCK ===
    if (p == WP || p == BP || m.captured != NONE)
        board.halfmoveClock = 0;
    else
        board.halfmoveClock++;

    // === SWITCH SIDE ===
    board.sideToMove = (board.sideToMove == WHITE) ? BLACK : WHITE;
    board.hash ^= zobristSide;

    u64 full = computeHash(board, board.sideToMove);

    // Update pst
    score -= pst[p][from];
    score += pst[p][to];

    board.eval += score;



    if (board.hash != full) {
        std::cout << "HASH MISMATCH\n";
        std::cout << "Incremental: " << board.hash << "\n";
        std::cout << "Full:        " << full << "\n";
        std::cout << "EP: " << board.enPassantSquare << "\n";
        std::cout << "Castling: " << board.castlingRights << "\n";
        std::cout << "Side: " << board.sideToMove << "\n";     
        exit(1);
    } 
    
    if (((board.all_white & board.all_black) != 0) && ((board.all_white | board.all_black) == board.all)) {
        std::cout << "==== STATE CORRUPTION DETECTED ====\n";
        printBoardAsLetters(board, false);

        for (int p = 0; p < 12; p++) {
            std::cout << "Piece " << p << ":\n";
            print_bitboard(board.pieces[p]);
        }

        std::cout << "ALL WHITE:\n";
        print_bitboard(board.all_white);

        std::cout << "ALL BLACK:\n";
        print_bitboard(board.all_black);

        std::cout << "ALL:\n";
        print_bitboard(board.all);
        exit(1);
    }
    for (int p = WP; p <= BK; p++) {
        if (board.pieces[p] & board.all_white && board.pieces[p] & board.all_black) {
            std::cout << "Piece appears on both sides bitboards!\n";
            exit(1);
        }
    }
    
    printBoardAsLetters(board, false);
    return u;
}



Undo make_null_move(Board& board);

void undo_null_move(Board& board, Undo u);


