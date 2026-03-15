#include <iostream>
#include <bitset>
#include <cstdint>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include "constants.h"
#include "moveGen.h"
#include "misc.h"
#include "makeMove.h"

using u64 = uint64_t;



void generateLegalMoves (Board& board, Color side, std::vector<Move>& moves) {
   
    moves.clear();
    
    generatePseudoLegalMoves(board, side, moves);

    


    int legalCount = 0;

    for (int i = 0; i < moves.size(); i++) {

        


        
        Board before = board;
        Undo u = makeMove(moves[i], board, side);

        if (!(isKingInCheck(side, board))) {
            moves[legalCount++] = moves[i];  // keep move
        }

        undoMove(moves[i], board, side, u);
        if (memcmp(&before, &board, sizeof(Board)) != 0) {
            logToFile("UNDO BROKEN!");
        }
    }

    moves.resize(legalCount);




    // --- Castling ---
//    u64 allPieces = board.all_white | board.all_black;
//     if (side == WHITE)
//     {
//         // White kingside (E1 -> G1, rook H1 -> F1)
//         if (canCastleKingside_white &&
//             !(allPieces & ((1ULL << 5) | (1ULL << 6))) && // F1, G1 empty
//             !isKingInCheck(WHITE, board))
//         {

//             moves.push_back(mkMove(4, 6, 7, 5, 0ULL, NONE)); // from2=7 (rook), to2=5 (rook move)
//         }

//         // White queenside (E1 -> C1, rook A1 -> D1)
//         if (canCastleQueenside_white &&
//             !(allPieces & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) && // B1, C1, D1 empty
//             !isKingInCheck(WHITE, board))
//         {

//             Board temp = board;
//             movePiece(temp.king_white, 4, 3, WHITE, temp);
//             if (!isKingInCheck(WHITE, temp))
//             {
//                 movePiece(temp.king_white, 3, 2, WHITE, temp);
//                 if (!isKingInCheck(WHITE, temp))
//                     moves.push_back(mkMove(4, 2, 0, 3, 0)); // from2=0 (rook), to2=3 (rook move)
//                                                             // std::cout << "move: " << fromSq << " → " << "K" << "\n";
//             }
//         }
//     }
//     else
//     {
//         // Black kingside (E8 -> G8, rook H8 -> F8)
//         if (canCastleKingside_black &&
//             !(allPieces & ((1ULL << 61) | (1ULL << 62))) && // F8, G8 empty
//             !isKingInCheck(BLACK, board))
//         {

//             Board temp = board;
//             movePiece(temp.king_black, 60, 61, BLACK, temp);
//             if (!isKingInCheck(BLACK, temp))
//             {
//                 movePiece(temp.king_black, 61, 62, BLACK, temp);
//                 if (!isKingInCheck(BLACK, temp))
//                     moves.push_back(mkMove(60, 62, 63, 61, 0));
//                 // std::cout << "move: " << fromSq << " → "  << "K" << "\n";
//             }
//         }

//         // Black queenside (E8 -> C8, rook A8 -> D8)
//         if (canCastleQueenside_black &&
//             !(allPieces & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
//             !isKingInCheck(BLACK, board))
//         {

//             Board temp = board;
//             movePiece(temp.king_black, 60, 59, BLACK, temp);
//             if (!isKingInCheck(BLACK, temp))
//             {
//                 movePiece(temp.king_black, 59, 58, BLACK, temp);
//                 if (!isKingInCheck(BLACK, temp))
//                     moves.push_back(mkMove(60, 58, 56, 59, 0));
//                 // std::cout << "move: " << fromSq << " → "  << "K" << "\n";
//             }
//         }
//     }

    
}

bool isSquareAttacked(const Board& board, Color side, int attackedSquare) {
    u64 occ = board.all_white | board.all_black; 
    Color attackingSide = (side == WHITE) ? BLACK : WHITE;
    u64 attackedSquareBB = 1ULL << attackedSquare;


    //Pawns
    if (attackingSide == WHITE) {
        
        u64 pawns = board.pawns_white;
        u64 pawnAttacks = pawns << 7 | pawns << 9;
        if(pawnAttacks & attackedSquareBB) {
            std::cerr << "Pawn" << std::endl;
            return true;
        }
    } 
    else if (attackingSide == BLACK) {
       
        u64 pawns = board.pawns_black;
        u64 pawnAttacks = pawns >> 7 | pawns >> 9;
        if(pawnAttacks & attackedSquareBB) {
            std::cerr << "Pawn" << std::endl;
            return true;
        } 
    }

    //Knights
    u64 knights = (attackingSide == WHITE) ? board.knights_white : board.knights_black;
    
    
    while (knights) {
        int fromSq = lsb(knights);
        knights &= knights - 1;

        if (knightAttacks[fromSq] & attackedSquareBB) {
            printBoardAsLetters(board, false);
            
            std::cerr << "Knight" << std::endl;
            return true;
        }
    }

    //King
    u64 king = (attackingSide == WHITE) ? board.king_white : board.king_black;
    
    int fromSq = lsb(king);

    if (kingAttacks[fromSq] & attackedSquareBB) {
        std::cerr << "King" << std::endl;
        printBoardAsLetters(board, false);

        return true;
    }


    //Bishops
    u64 bishops = (attackingSide == WHITE) ? board.bishops_white : board.bishops_black;
    
    while (bishops) {
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 bishopAttacks = getBishopAttackMagics(fromSq, occ);

        if (bishopAttacks & attackedSquareBB) {
            std::cerr << "Bishop" << std::endl;
            return true;
        }
    }

    //Rooks
    u64 rooks = (attackingSide == WHITE) ? board.rooks_white : board.rooks_black;
    
    while (rooks) {
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 rookAttacks = getRookAttackMagics(fromSq, occ);

        if (rookAttacks & attackedSquareBB) {
            std::cerr << "Rook" << std::endl;
            return true;
        }
    }

    //Queens
    u64 queens = (attackingSide == WHITE) ? board.queens_white : board.queens_black;
    
    while (queens) {
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 queenAttacks = getBishopAttackMagics(fromSq, occ) | getRookAttackMagics(fromSq, occ);

        if (queenAttacks & attackedSquareBB) {
            std::cerr << "Queen" << std::endl;
            return true;
        }
    }

    return false;
}

bool isKingInCheck(Color side, const Board &board) {
    u64 occ = board.all_white | board.all_black;
    u64 kingBB = (side == WHITE) ? board.king_white : board.king_black;
    
    
    if (!kingBB) {
        std::cerr << "isKingInCheck: No King " << std::endl;
        return false;
    }

    int kingSq = lsb(kingBB);


    if (isSquareAttacked(board, side, kingSq)) {
        std::cerr << "King In check" << side << std::endl;
        printBoardAsLetters(board, true);
      return true;
    }

    return false;
}

