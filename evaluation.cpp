#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include "search.h"

using u64 = uint64_t;

int evaluate(Board& board, Color side, std::vector<Move>& moves) {
    // === Generate all legal moves for the current side ===
    moves.clear();
    generatePawnMoves(board, side, moves);
    generateKnightMoves(board, side, moves);
    generateBishopMoves(board, side, moves);
    generateRookMoves(board, side, moves);
    generateQueenMoves(board, side, moves);
    generateKingMoves(board, side, moves);

    // Filter only legal moves (that don't leave king in check)
    std::vector<Move> legalMoves;
    for (auto &m : moves) {
        Undo u = makeMove(m, board, side);
        bool illegal = isKingInCheck(side, board);
        std::cout << illegal;
        undoMove(m, board, side, u);
        if (!illegal) legalMoves.push_back(m);
    }

    Color oppSide = (side == WHITE) ? BLACK : WHITE;
    // === Generate all legal moves for the opposite side ===
    moves.clear();
    generatePawnMoves(board, oppSide, moves);
    generateKnightMoves(board, oppSide, moves);
    generateBishopMoves(board, oppSide, moves);
    generateRookMoves(board, oppSide, moves);
    generateQueenMoves(board, oppSide, moves);
    generateKingMoves(board, oppSide, moves);

    // Filter only legal moves (that don't leave king in check)
    std::vector<Move> oppLegalMoves;
    for (auto &m : moves) {
        Undo u = makeMove(m, board, oppSide);
        bool illegal = isKingInCheck(oppSide, board);
        std::cout << illegal;
        undoMove(m, board, oppSide, u);
        if (!illegal) oppLegalMoves.push_back(m);
    }

    // === Checkmate and stalemate detection ===
    if (legalMoves.empty()) {
        if (isKingInCheck(side, board)) {
            // Checkmate for the current side
            return (side == WHITE) ? -1000000 : 1000000;
        } else {
            // Stalemate
            return 0;
        }
    }

    // === Checkmate and stalemate detection ===
    if (oppLegalMoves.empty()) {
        if (isKingInCheck(oppSide, board)) {
            // Checkmate for the current side
            return (side == WHITE) ? 1000000 : -1000000;
        } else {
            // Stalemate
            return 0;
        }
    }

    // === If we reach here, it's a normal position ===
    // You can now use your usual evaluation (material + positional)
    int score = 0;

    // --- Material count ---
    score += popcount(board.pawns_white)   * PAWN_VALUE;
    score += popcount(board.knights_white) * KNIGHT_VALUE;
    score += popcount(board.bishops_white) * BISHOP_VALUE;
    score += popcount(board.rooks_white)   * ROOK_VALUE;
    score += popcount(board.queens_white)  * QUEEN_VALUE;

    score -= popcount(board.pawns_black)   * PAWN_VALUE;
    score -= popcount(board.knights_black) * KNIGHT_VALUE;
    score -= popcount(board.bishops_black) * BISHOP_VALUE;
    score -= popcount(board.rooks_black)   * ROOK_VALUE;
    score -= popcount(board.queens_black)  * QUEEN_VALUE;

    // === Add slight bonus/penalty for being in check ===
    if (isKingInCheck(side, board)) {
        score += (side == WHITE) ? -50 : 50;
    }

    return score;
}