#include <iostream>
#include <bitset>
#include <cstdint>
#include <vector>
#include <string>
#include <bits/stdc++.h>
#include <cstdlib>
#include "constants.h"
#include "moveGen.h"
#include "misc.h"
#include "makeMove.h"

using u64 = uint64_t;


bool isMoveLegal(Board& board, Move m, Color side) {
    Undo u = makeMove(m, board);
    bool legal = false;
        

    if (!(isKingInCheck(side, board))) {
        legal = true;
    }
        

        

    undoMove(m, board, u);
    return legal;
}

int generateLegalMoves (Board& board, Color side, Move* moves) {
   
    
    int count = generatePseudoLegalMoves(board, side, moves);

    

    int legalCount = 0;

    for (int i = 0; i < count; i++) {

    
        if (getPieceType(board, moves[i].from) == NONE) {
            std::cout << "INVALID MOVE: from=" << moves[i].from << "\n";
            exit(1);
        }
        Board before = board;
        
        

        if (isMoveLegal(board, moves[i], side)) {
            moves[legalCount++] = moves[i];  // keep move
        }
        

        

        if (memcmp(&before, &board, sizeof(Board)) != 0) {
            
            sameBoard(before, board);
            std::cout << "attack: UNDO BROKEN\n";
            exit(1);
        }

    }

    return legalCount;
} 

bool isSquareAttacked(const Board& board, Color side, int attackedSquare) {
    u64 occ = board.all_white | board.all_black; 
    Color attackingSide = (side == WHITE) ? BLACK : WHITE;
    u64 attackedSquareBB = 1ULL << attackedSquare;


    //Pawns
    if (attackingSide == WHITE) {
        
        u64 pawns = board.pieces[WP];
        u64 pawnAttacks = ((pawns << 7) & ~FILE_H) | ((pawns << 9) & ~FILE_A);
        if(pawnAttacks & attackedSquareBB) {
            return true;
        }
    } 
    else if (attackingSide == BLACK) {
       
        u64 pawns = board.pieces[BP];
        u64 pawnAttacks = ((pawns >> 7) & ~FILE_A) | ((pawns >> 9) & ~FILE_H);
        if(pawnAttacks & attackedSquareBB) {
            return true;
        } 
    }

    //Knights
    u64 knights = (attackingSide == WHITE) ? board.pieces[WN] : board.pieces[BN];
    
    
    while (knights) {
        int fromSq = lsb(knights);
        knights &= knights - 1;

        if (knightAttacks[fromSq] & attackedSquareBB) {
            
            return true;
        }
    }

    //King
    u64 king = (attackingSide == WHITE) ? board.pieces[WK] : board.pieces[BK];
    
    int fromSq = lsb(king);

    if (kingAttacks[fromSq] & attackedSquareBB) {

        return true;
    }


    //Bishops
    u64 bishops = (attackingSide == WHITE) ? board.pieces[WB] : board.pieces[BB];
    
    while (bishops) {
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 bishopAttacks = getBishopAttackMagics(fromSq, occ);

        if (bishopAttacks & attackedSquareBB) {
            return true;
        }
    }

    //Rooks
    u64 rooks = (attackingSide == WHITE) ? board.pieces[WR] : board.pieces[BR];
    
    while (rooks) {
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 rookAttacks = getRookAttackMagics(fromSq, occ);

        if (rookAttacks & attackedSquareBB) {
            return true;
        }
    }

    //Queens
    u64 queens = (attackingSide == WHITE) ? board.pieces[WQ] : board.pieces[BQ];
    
    while (queens) {
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 queenAttacks = getQueenAttackMagics(fromSq, occ);
    

        if (queenAttacks & attackedSquareBB) {
            return true;
        }
    }

    return false;
}

bool isKingInCheck(Color side, const Board &board) {
    u64 occ = board.all_white | board.all_black;
    u64 kingBB = (side == WHITE) ? board.pieces[WK] : board.pieces[BK];
    
    
    if (!kingBB) {
        std::cerr << "isKingInCheck: No King " << std::endl;
        printBoardAsLetters(board, false);
        return false;
    }

    int kingSq = lsb(kingBB);


    if (isSquareAttacked(board, side, kingSq)) {
      return true;
    }

    return false;
}

int generateCaptures(Board& board, Color side, Move* moves) {
    u64 oppSideBB = (side == WHITE) ? board.all_black : board.all_white;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;
    int count = 0;


    //PAWNS
    u64 pawns = (side == WHITE) ? board.pieces[WP] : board.pieces[BP];
    u64 originalPawns = pawns;

    while (pawns) {
        int fromSq = lsb(pawns);
        
        pawns &= pawns - 1;
        int toSq;
        u64 fromSqBB = 1ULL << fromSq;

        if (side == WHITE) {
            if ((fromSqBB & ~FILE_A)) {
                toSq = fromSq + 7;
                if (toSq >= 0 && toSq < 64) {
                    if (oppSideBB & (1ULL << toSq)) {
                        if (fromSqBB & RANK_7) {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WN, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WB, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WR, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WQ, CAPTURE | PROMOTION);

                        } else {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), NONE, CAPTURE);
                        }
                    }
                }
                    
            }

            if ((fromSqBB & ~FILE_H)) {
                toSq = fromSq + 9;
                if (toSq >= 0 && toSq < 64) {
                    if (oppSideBB & (1ULL << toSq)) {
                        if (fromSqBB & RANK_7) {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WN, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WB, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WR, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), WQ, CAPTURE | PROMOTION);

                        } else {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, WP, getPieceType(board, toSq), NONE, CAPTURE);
                        }
                    }
                }
                    
            }
        } else {
            if ((fromSqBB & ~FILE_H)) {
                toSq = fromSq - 7;
                if (toSq >= 0 && toSq < 64) {
                    if (oppSideBB & (1ULL << toSq)) {
                        if (fromSqBB & RANK_2) {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BN, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BB, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BR, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BQ, CAPTURE | PROMOTION);

                        } else {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), NONE, CAPTURE);
                        }
                    }
                }
                    
            }

            if ((fromSqBB & ~FILE_A)) {
                toSq = fromSq - 9;
                if (toSq >= 0 && toSq < 64) {
                    if (oppSideBB & (1ULL << toSq)) {
                        if (fromSqBB & RANK_2) {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BN, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BB, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BR, CAPTURE | PROMOTION);
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), BQ, CAPTURE | PROMOTION);

                        } else {
                            moves[count++] = mkMove(fromSq, toSq, -1, -1, BP, getPieceType(board, toSq), NONE, CAPTURE);
                        }
                    }
                }
                    
            }
        }

        if (board.enPassantSquare != -1) {
            int epSq = board.enPassantSquare;
            int fromEp;

            if (side == WHITE) {
                if (epSq % 8 != 0) {  // left EP
                    fromEp = epSq - 9;
                    if ((fromEp >= 0 && fromEp < 64) && (originalPawns & (1ULL << fromEp))) {
                        moves[count++] = mkMove(fromEp, epSq, -1, -1, WP, BP, NONE, CAPTURE | ENPASSANT);
                    }
                }
                if (epSq % 8 != 7) {  // right EP
                    fromEp = epSq - 7;
                    if (fromEp >= 0 && fromEp < 64 && (originalPawns & (1ULL << fromEp))) {
                        moves[count++] = mkMove(fromEp, epSq, -1, -1, WP, BP, NONE, CAPTURE | ENPASSANT);
                    }
                }
            } else {  // black
                if (epSq % 8 != 0) {  // left EP
                    fromEp = epSq + 7;
                    if (fromEp >= 0 && fromEp < 64 && (originalPawns & (1ULL << fromEp))) { 
                        moves[count++] = mkMove(fromEp, epSq, -1, -1, BP, WP, NONE, CAPTURE | ENPASSANT);
                    }
                }
                if (epSq % 8 != 7) {  // right EP
                    fromEp = epSq + 9;
                    if (fromEp >= 0 && fromEp < 64 && (originalPawns & (1ULL << fromEp))) {
                        moves[count++] = mkMove(fromEp, epSq, -1, -1, BP, WP, NONE, CAPTURE | ENPASSANT);
                    }
                }
            }
        }
        

    }

    //KNIGHTS
    u64 knights = (side == WHITE) ? board.pieces[WN] : board.pieces[BN];

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;


        u64 captures = knightAttacks[fromSq] & ~ownPieces & oppSideBB;

        while (captures)
        {
            int toSq = lsb(captures);
            captures &= captures - 1;
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (side == WHITE) ? WN : BN, getPieceType(board, toSq), NONE, CAPTURE);
        }
    }

    //BISHOPS
    u64 occ = oppSideBB | ownPieces;
    u64 bishops = (side == WHITE) ? board.pieces[WB] : board.pieces[BB];

    while (bishops)
    {
        
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 captures = getBishopAttackMagics(fromSq, occ);
        captures &= ~ownPieces & oppSideBB;

        while (captures)
        {
            int toSq = lsb(captures);
            captures &= captures - 1;
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (side == WHITE) ? WB : BB, getPieceType(board, toSq), NONE, CAPTURE);
        }
    }

    //ROOKS
    u64 rooks = (side == WHITE) ? board.pieces[WR] : board.pieces[BR];

    while (rooks)
    {
        
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 captures = getRookAttackMagics(fromSq, occ);
        captures &= ~ownPieces & oppSideBB;

        while (captures)
        {
            int toSq = lsb(captures);
            captures &= captures - 1;
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (side == WHITE) ? WR : BR, getPieceType(board, toSq), NONE, CAPTURE);
        }
    }

    //QUEENS
    u64 queens = (side == WHITE) ? board.pieces[WQ] : board.pieces[BQ];

    while (queens)
    {
        
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 captures = getBishopAttackMagics(fromSq, occ) | getRookAttackMagics(fromSq, occ);
        captures &= ~ownPieces & oppSideBB;

        while (captures)
        {
            int toSq = lsb(captures);
            captures &= captures - 1;
            moves[count++] = mkMove(fromSq, toSq, -1, -1, (side == WHITE) ? WQ : BQ, getPieceType(board, toSq), NONE, CAPTURE);
        }
    }

    return count;
}


int generateQuietMoves(Board& board, Move* moves) {
    int moveCount = generateLegalMoves(board, board.sideToMove, moves);
    int quietCount = 0;

    for (int i = 0; i < moveCount; i++) {
        Move m = moves[i];

        if (m.flags & QUIET) 
            moves[quietCount++] = moves[i];

        
    }

    return quietCount;
}