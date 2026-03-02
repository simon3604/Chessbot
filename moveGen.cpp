#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include "constants.h"
#include "moveGen.h"

using u64 = uint64_t;


u64 getPawnPushes(u64 bit, Color side, const Board &board)
{
    u64 occ = board.all_white | board.all_black;
    u64 moves = 0ULL;

    if (side == WHITE)
    {

        u64 oneAhead = bit << 8;
        if (!(oneAhead & occ))
        {
            moves |= oneAhead;
            if ((bit & RANK_2) && !(oneAhead << 8 & occ))
                moves |= oneAhead << 8;
        }
    }
    else
    {

        u64 oneAhead = bit >> 8;
        if (!(oneAhead & occ))
        {
            moves |= oneAhead;
            if ((bit & RANK_7) && !(oneAhead >> 8 & occ))
                moves |= oneAhead >> 8;
        }
    }
    return moves;
}

u64 getPawnCaptures(u64 bit, Color side, const Board &board)
{
    u64 moves = 0ULL;
    if (side == WHITE)
    {
        moves |= (bit << 7) & ~FILE_H & board.all_black;
        moves |= (bit << 9) & ~FILE_A & board.all_black;
    }
    else
    {
        moves |= (bit >> 7) & ~FILE_A & board.all_white;
        moves |= (bit >> 9) & ~FILE_H & board.all_white;
    }
    return moves;
}

void generatePawnMoves(const Board& board, Color Side, std::vector<Move>& moves) {
    u64 pawns = (side == WHITE) ? board.pawns_white : board.pawns_black;
    u64 empty = ~(board.all_white | board.all_black);

    while (pawns) {
        int fromSq = lsb(pawns);
        pawns &= pawns - 1;
        
        u64 pushTargets = getPawnPushes(1ULL << fromSq, side, board);
        u64 capTargets = getPawnCaptures(1ULL << fromSq, side, board);
        int ep = board.enPassantSquare;
        
        // === Handle pawn pushes ===
        while (pushTargets)
        {
            
            int toSq = lsb(pushTargets);
            pushTargets &= pushTargets - 1;

            // If the target square is the last rank -> promotion
            if ((side == WHITE && (1ULL << toSq) & RANK_8) ||
                (side == BLACK && (1ULL << toSq) & RANK_1))
            {

                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, KNIGHT));
                std::cerr << "promotion push: " << fromSq << " → " << toSq << "\n";
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
                // std::cout << "move: " << fromSq << " → " << toSq << "\n";
            }
        }

        // === Handle captures ===
        while (capTargets)
        {
            int toSq = lsb(capTargets);
            capTargets &= capTargets - 1;

            if ((side == WHITE && (1ULL << toSq) & RANK_8) ||
                (side == BLACK && (1ULL << toSq) & RANK_1))
            {

                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, QUEEN));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, ROOK));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, BISHOP));
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, KNIGHT));
                std::cerr << "promotion capture: " << fromSq << " → " << toSq << "\n";
                
                
            }
            else
            {
                moves.push_back(mkMove(fromSq, toSq, -1, -1, 0ULL, NONE));
                // std::cout << "move: " << fromSq << " → " << toSq << "\n";
            }
        }

        //En Passant
        if (ep != -1) {
            u64 epBit = 1ULL << ep;

            if (side == WHITE) {
                if (fromSq % 8 != 0 && epBit == (1ULL << (fromSq + 7)))  // capture left
                    moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep - 8)));
                if (fromSq % 8 != 7 && epBit == (1ULL << (fromSq + 9)))  // capture right
                    moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep - 8)));
            } else {
                if (fromSq % 8 != 0 && epBit == (1ULL << (fromSq - 9)))
                    moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep + 8)));
                if (fromSq % 8 != 7 && epBit == (1ULL << (fromSq - 7)))
                    moves.push_back(mkMove(fromSq, ep, -1, -1, 1ULL << (ep + 8)));
            }
        }
    }
};

void generateKnightMoves(const Board& board, Color Side, std::vector<Move>& moves) {

    u64 knights = (side == WHITE) ? board.knights_white : board.knights_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;


        u64 attacks = 0ULL;
        u64 bit = 1ULL << fromSq;

        attacks |= (bit << 17) & ~FILE_A;
        attacks |= (bit << 15) & ~FILE_H;
        attacks |= (bit << 10) & ~(FILE_A | FILE_B);
        attacks |= (bit << 6) & ~(FILE_H | FILE_G);
        attacks |= (bit >> 17) & ~FILE_H;
        attacks |= (bit >> 15) & ~FILE_A;
        attacks |= (bit >> 10) & ~(FILE_H | FILE_G);
        attacks |= (bit >> 6) & ~(FILE_A | FILE_B);

        attacks &= ~ownPieces; // can't capture own pieces


        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(fromSq, toSq, -1, -1, 0ULL, NONE);
        }
    }

    

    return moves;
};

void generateBishopMoves(const Board& board, Color Side, std::vector<Move>& moves);
void generateRookMoves(const Board& board, Color Side, std::vector<Move>& moves);
void generateQueenMoves(const Board& board, Color Side, std::vector<Move>& moves);
void generateKingMoves(const Board& board, Color Side, std::vector<Move>& moves);