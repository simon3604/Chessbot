#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include "constants.cpp"
#include "generateMoves.cpp"

using u64 = uint64_t;

//Old makeMove function
u64 movePiece(u64 &pieceBB, int fromSq, int toSq, Color side, Board &board) {
    u64 captured = 0ULL;

    if (side == WHITE) {
        if ((1ULL << toSq) & board.all_black) {
            u64* captureBB = getTypeBB(toSq, board);
            if (captureBB) {
                captured = *captureBB & (1ULL << toSq);
                *captureBB &= ~(1ULL << toSq);
            }
        }
    } else {
        if ((1ULL << toSq) & board.all_white) {
            u64* captureBB = getTypeBB(toSq, board);
            if (captureBB) {
                captured = *captureBB & (1ULL << toSq);
                *captureBB &= ~(1ULL << toSq);
            }
        }
    }

    // move the piece
    pieceBB &= ~(1ULL << fromSq);
    pieceBB |= (1ULL << toSq);

    // update all_white/all_black
    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white | board.queens_white | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black | board.queens_black | board.king_black;

    return captured;
}


u64* getTypeBB(int pos, Board& board) {
    u64 mask = 1ULL << pos;
    if (mask & board.pawns_white)  return &board.pawns_white;
    if (mask & board.rooks_white)  return &board.rooks_white;
    if (mask & board.knights_white) return &board.knights_white;
    if (mask & board.bishops_white) return &board.bishops_white;
    if (mask & board.queens_white)  return &board.queens_white;
    if (mask & board.king_white)   return &board.king_white;

    if (mask & board.pawns_black)  return &board.pawns_black;
    if (mask & board.rooks_black)  return &board.rooks_black;
    if (mask & board.knights_black) return &board.knights_black;
    if (mask & board.bishops_black) return &board.bishops_black;
    if (mask & board.queens_black)  return &board.queens_black;
    if (mask & board.king_black)   return &board.king_black;

    return nullptr;
}


void check_board_integrity(const Board &board, int square, const char* tag = "") {
    u64 all = 0ULL;
    std::string prefix = tag ? std::string(tag) + ": " : "";
    auto check_and_report = [&](u64 bb, const char* name) {
        if (bb & all) {
            std::cerr << square << prefix << "OVERLAP detected for " << name << "\n";
        }
        all |= bb;
    };

    check_and_report(board.pawns_white,   "pawns_white");
    check_and_report(board.knights_white, "knights_white");
    check_and_report(board.bishops_white, "bishops_white");
    check_and_report(board.rooks_white,   "rooks_white");
    check_and_report(board.queens_white,  "queens_white");
    check_and_report(board.king_white,    "king_white");

    check_and_report(board.pawns_black,   "pawns_black");
    check_and_report(board.knights_black, "knights_black");
    check_and_report(board.bishops_black, "bishops_black");
    check_and_report(board.rooks_black,   "rooks_black");
    check_and_report(board.queens_black,  "queens_black");
    check_and_report(board.king_black,    "king_black");

    // check all_white/all_black are consistent:
    u64 recomputed_white = board.pawns_white | board.knights_white | board.bishops_white |
                           board.rooks_white  | board.queens_white  | board.king_white;
    u64 recomputed_black = board.pawns_black | board.knights_black | board.bishops_black |
                           board.rooks_black  | board.queens_black  | board.king_black;

    if (recomputed_white != board.all_white)
        std::cerr << prefix << "all_white mismatch: recomputed=" << std::hex << recomputed_white
                  << " stored=" << board.all_white << std::dec << "\n";
    if (recomputed_black != board.all_black)
        std::cerr << prefix << "all_black mismatch: recomputed=" << std::hex << recomputed_black
                  << " stored=" << board.all_black << std::dec << "\n";

    // check bit 0 specifically
    // bool bit0set = ((board.queens_white | board.rooks_white | board.pawns_white |
    //                  board.knights_white | board.bishops_white) & 1ULL) != 0;
    // if (bit0set) {
    //     std::cerr << prefix << "bit 0 is set in white piece bitboards!\n";
    //     print_bitboard(board.queens_white);
    //     print_bitboard(board.rooks_white);
    //     print_bitboard(board.pawns_white);
    // }
}


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
        *pawnBB |= (1ULL << m.from);
    } else {
        // Normal move undo: move the bit back from 'to' -> 'from'
        if (undo.movedPiece) {
            *undo.movedPiece &= ~(1ULL << m.to);
            *undo.movedPiece |=  (1ULL << m.from);
        }
    }

    // Restore captured piece (if any)
    if (undo.captured && undo.capturedPiece) {
        *undo.capturedPiece |= undo.captured;
    }

    // Undo rook move from castling
    if (undo.movedRook) {
        *undo.movedRook &= ~(1ULL << m.to2);
        *undo.movedRook |=  (1ULL << m.from2);
    }

    // Recompute occupancy masks (always do this after changes)
    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white  | board.queens_white  | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black  | board.queens_black  | board.king_black;
                    
    check_board_integrity(board, m.to);              
}


// Make a move on the board and return undo info
Undo makeMove(Move m, Board& board, Color side) {
    Undo undo;
    undo.from = m.from;
    undo.to = m.to;
    undo.from2 = m.from2;
    undo.to2 = m.to2;
    undo.movedPiece = getTypeBB(m.from, board);
    undo.captured = 0ULL;
    undo.capturedPiece = nullptr;
    undo.movedRook = nullptr;

    if (!undo.movedPiece) return undo; // safety check

    // handle capture
    u64 targetMask = 1ULL << m.to;

    if (side == WHITE && (targetMask & board.all_black)) {
        undo.capturedPiece = getTypeBB(m.to, board);
        if (undo.capturedPiece) {
            undo.captured = targetMask;
            *undo.capturedPiece &= ~targetMask;
        }
    } else if (side == BLACK && (targetMask & board.all_white)) {
        undo.capturedPiece = getTypeBB(m.to, board);
        if (undo.capturedPiece) {
            undo.captured = targetMask;
            *undo.capturedPiece &= ~targetMask;
        }
    }


    if ((m.promotion != NONE) && !((1ULL << m.from) & ((side == WHITE) ? board.pawns_white : board.pawns_black))) {
    std::cerr << "⚠️ ERROR: Promotion set on a non-pawn move! from=" << m.from
              << " to=" << m.to << "\n";
    }
    

    u64 maskFrom = 1ULL << m.from;
    bool isPawn = (side == WHITE)
        ? (maskFrom & board.pawns_white)
        : (maskFrom & board.pawns_black);

    // std::cerr << "makeMove: from " << m.from
    //         << " to " << m.to
    //         << " promo=" << m.promotion
    //         << " isPawn=" << isPawn
    //         << "\n";


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

    // === Handle castling rook ===
    if (m.from2 != -1 && m.to2 != -1) {
        undo.movedRook = getTypeBB(m.from2, board);
        if (undo.movedRook) {
            *undo.movedRook &= ~(1ULL << m.from2);
            *undo.movedRook |=  (1ULL << m.to2);
        }
    }

    if (isKingInCheck(side, board)) {
        undoMove(m, board, side, undo);
        undo.to = -1;
        return undo;
    }

    // === Update bitboards ===
    board.all_white =
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black =
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;


    check_board_integrity(board, m.to);
    return undo;
}
