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
using u32 = uint32_t;

bool isMoveLegal(Board &board, u32 m, std::string debugInfo)
{
    Piece piece = getPieceType(board, fromSquare(m));
    Color side = ((piece <= WK) && (piece >= WP)) ? WHITE : BLACK;

    // 1. Piece must exist
    if (piece == NONE)
        return false;

    // 2. Can't capture own piece
    Piece target = getPieceType(board, toSquare(m));
    if (target != NONE)
    {
        Color targetColor = pieceColor(target, "isMoveLegal");
        if (targetColor == board.sideToMove)
            return false;
    }

    // 3. Can't move to the same square
    if (fromSquare(m) == toSquare(m))
        return false;

    // 4. Can't move a piece that doesn't belong to the side to move
    if (side != board.sideToMove)
        return false;

    // 5. Capture consistency
    Piece capturedPiece = (moveFlags(m) & CAPTURE) ? target : NONE;

    if (capturedPiece == NONE && target != NONE)
        return false;
    
    if (capturedPiece != NONE && target == NONE && ((moveFlags(m) & ENPASSANT) == 0))
    {
        return false;
    }

    if (moveFlags(m) & ENPASSANT)
    {

        if (toSquare(m) != board.enPassantSquare)
        {
            return false;
        }
    }

    // 6. Make move and test check
    Undo u = makeMove(m, board, "isMoveLegal, " + debugInfo);
    bool illegal = isKingInCheck(side, board, "isMoveLegal, " + debugInfo);

    undoMove(m, board, u, "isMoveLegal, " + debugInfo);

    return !illegal;
}

int generateLegalMoves(Board &board, Color side, u32 *moves)
{

    int count = generatePseudoLegalMoves(board, side, moves);

    int legalCount = 0;

    for (int i = 0; i < count; i++)
    {

        if (getPieceType(board, fromSquare(moves[i])) == NONE)
        {
            std::cerr << "INVALID MOVE: from=" << fromSquare(moves[i]) << "\n";
            exit(1);
        }
        Board before = board;

        if (isMoveLegal(board, moves[i], "generateLegalMoves"))
        {
            moves[legalCount++] = moves[i]; // keep move
        }

        
    }

    return legalCount;
}

bool isSquareAttacked(const Board &board, Color side, int attackedSquare)
{
    Color attackingSide = (side == WHITE) ? BLACK : WHITE;
    u64 attackedSquareBB = 1ULL << attackedSquare;

    // Pawns
    if (attackingSide == WHITE)
    {

        u64 pawns = board.pieces[WP];
        u64 pawnAttacks = ((pawns << 7) & ~FILE_H) | ((pawns << 9) & ~FILE_A);
        if (pawnAttacks & attackedSquareBB)
        {
            return true;
        }
    }
    else if (attackingSide == BLACK)
    {

        u64 pawns = board.pieces[BP];
        u64 pawnAttacks = ((pawns >> 7) & ~FILE_A) | ((pawns >> 9) & ~FILE_H);
        if (pawnAttacks & attackedSquareBB)
        {
            return true;
        }
    }

    // Knights
    u64 knights = (attackingSide == WHITE) ? board.pieces[WN] : board.pieces[BN];

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;

        if (knightAttacks[fromSq] & attackedSquareBB)
        {

            return true;
        }
    }

    // King
    u64 king = (attackingSide == WHITE) ? board.pieces[WK] : board.pieces[BK];

    int fromSq = lsb(king);

    if (kingAttacks[fromSq] & attackedSquareBB)
    {

        return true;
    }

    // Bishops
    u64 bishops = (attackingSide == WHITE) ? board.pieces[WB] : board.pieces[BB];

    while (bishops)
    {
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 bishopAttacks = getBishopAttackMagics(fromSq, board.all);

        if (bishopAttacks & attackedSquareBB)
        {
            return true;
        }
    }

    // Rooks
    u64 rooks = (attackingSide == WHITE) ? board.pieces[WR] : board.pieces[BR];

    while (rooks)
    {
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 rookAttacks = getRookAttackMagics(fromSq, board.all);

        if (rookAttacks & attackedSquareBB)
        {
            return true;
        }
    }

    // Queens
    u64 queens = (attackingSide == WHITE) ? board.pieces[WQ] : board.pieces[BQ];

    while (queens)
    {
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 queenAttacks = getQueenAttackMagics(fromSq, board.all);

        if (queenAttacks & attackedSquareBB)
        {
            return true;
        }
    }

    return false;
}

bool isKingInCheck(Color side, const Board &board, std::string debugInfo)
{
    u64 kingBB = (side == WHITE) ? board.pieces[WK] : board.pieces[BK];

    if (!kingBB)
    {
        std::cerr << "isKingInCheck: No King: " << debugInfo << std::endl;
        printBoardAsLetters(board, false);
        exit(1);
    }

    int kingSq = lsb(kingBB);

    if (isSquareAttacked(board, side, kingSq))
    {
        return true;
    }

    return false;
}

int generateCaptures(Board &board, Color side, u32 *moves, std::string debugInfo)
{
    int count = 0;

    u64 empty = ~board.all;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;
    u64 oppPieces = (side == WHITE) ? board.all_black : board.all_white;

    //=== Pawns ===
    u64 pawns = (side == WHITE) ? board.pieces[WP] : board.pieces[BP];

    if (side == WHITE)
    {
        // === Captures left ===
        u64 leftCaps = (pawns << 7) & ~FILE_H & board.all_black;

        while (leftCaps)
        {
            int toSq = lsb(leftCaps);
            leftCaps &= leftCaps - 1;

            int fromSq = toSq - 7;

            if ((1ULL << toSq) & RANK_8)
            {
                moves[count++] = mkMove(fromSq, toSq, WQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, WR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, WB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, WN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }

        // === Captures right ===
        u64 rightCaps = (pawns << 9) & ~FILE_A & board.all_black;

        while (rightCaps)
        {
            int toSq = lsb(rightCaps);
            rightCaps &= rightCaps - 1;

            int fromSq = toSq - 9;

            if ((1ULL << toSq) & RANK_8)
            {
                moves[count++] = mkMove(fromSq, toSq, WQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, WR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, WB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, WN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }

        // En passant

        if (board.enPassantSquare != -1)
        {
            u64 epBB = 1ULL << board.enPassantSquare;

            // left EP (<<7)
            u64 epLeft = (pawns << 7) & ~FILE_H & epBB;

            while (epLeft)
            {
                int toSq = lsb(epLeft);
                epLeft &= epLeft - 1;

                int fromSq = toSq - 7;

                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE | ENPASSANT);
            }

            // right EP (<<9)
            u64 epRight = (pawns << 9) & ~FILE_A & epBB;

            while (epRight)
            {
                int toSq = lsb(epRight);
                epRight &= epRight - 1;

                int fromSq = toSq - 9;

                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE | ENPASSANT);
            }
        }
    }
    else
    {

        // === Captures left ===
        u64 leftCaps = (pawns >> 9) & ~FILE_H & board.all_white;

        while (leftCaps)
        {
            int toSq = lsb(leftCaps);
            leftCaps &= leftCaps - 1;

            int fromSq = toSq + 9;

            if ((1ULL << toSq) & RANK_1)
            {
                moves[count++] = mkMove(fromSq, toSq, BQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, BR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, BB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, BN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }

        // === Captures right ===
        u64 rightCaps = (pawns >> 7) & ~FILE_A & board.all_white;

        while (rightCaps)
        {
            int toSq = lsb(rightCaps);
            rightCaps &= rightCaps - 1;

            int fromSq = toSq + 7;

            if ((1ULL << toSq) & RANK_1)
            {
                moves[count++] = mkMove(fromSq, toSq, BQ, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, BR, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, BB, CAPTURE | PROMOTION);
                moves[count++] = mkMove(fromSq, toSq, BN, CAPTURE | PROMOTION);
            }
            else
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }

        // En passant

        if (board.enPassantSquare != -1)
        {
            u64 epBB = 1ULL << board.enPassantSquare;

            // left EP (>>9)
            u64 epLeft = (pawns >> 9) & ~FILE_H & epBB;

            while (epLeft)
            {
                int toSq = lsb(epLeft);
                epLeft &= epLeft - 1;

                int fromSq = toSq + 9;

                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE | ENPASSANT);
            }

            // right EP (>>7)
            u64 epRight = (pawns >> 7) & ~FILE_A & epBB;

            while (epRight)
            {
                int toSq = lsb(epRight);
                epRight &= epRight - 1;

                int fromSq = toSq + 7;

                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE | ENPASSANT);
            }
        }
    }

    //=== Knights ===
    u64 knights = (side == WHITE) ? board.pieces[WN] : board.pieces[BN];

    while (knights)
    {
        int fromSq = lsb(knights);
        knights &= knights - 1;

        u64 attacks = knightAttacks[fromSq] & ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;
            if ((1ULL << toSq) & oppPieces)
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }
    }

    //=== Rooks ===
    u64 rooks = (side == WHITE) ? board.pieces[WR] : board.pieces[BR];

    while (rooks)
    {

        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 attacks = getRookAttackMagics(fromSq, board.all);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;
            if ((1ULL << toSq) & oppPieces)
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }
    }

    //=== Bishops ===

    u64 bishops = (side == WHITE) ? board.pieces[WB] : board.pieces[BB];

    while (bishops)
    {

        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 attacks = getBishopAttackMagics(fromSq, board.all);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            Piece captured;
            if ((1ULL << toSq) & oppPieces)
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }
    }

    //=== Queens ===

    u64 queens = (side == WHITE) ? board.pieces[WQ] : board.pieces[BQ];

    while (queens)
    {
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 attacks = getQueenAttackMagics(fromSq, board.all);
        attacks &= ~ownPieces;

        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;

            if ((1ULL << toSq) & oppPieces)
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }
    }

    //=== King ===

    u64 king = (side == WHITE) ? board.pieces[WK] : board.pieces[BK];

    if (king)
    {

        int fromSq = lsb(king);
        u64 attacks = kingAttacks[fromSq] & ~ownPieces;

        // Normal king moves
        while (attacks)
        {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;

            if ((1ULL << toSq) & oppPieces)
            {
                moves[count++] = mkMove(fromSq, toSq, NONE, CAPTURE);
            }
        }
    }

    if (count >= 256)
    {
        std::cerr << "OVERFLOW inside generator\n";
        exit(1);
    }
    Color oldSide = board.sideToMove;
    board.sideToMove = side;

    int legalCount = 0;
    for (int i = 0; i < count; i++)
    {
        u32 m = moves[i];

        if (isMoveLegal(board, m, "generateCaptures"))
            moves[legalCount++] = moves[i];
    }

    board.sideToMove = oldSide;
    return legalCount;
}

int generateQuietMoves(Board &board, Color side, u32 *moves)
{
    int moveCount = generateLegalMoves(board, board.sideToMove, moves);
    int quietCount = 0;

    for (int i = 0; i < moveCount; i++)
    {
        u32 m = moves[i];

        if (moveFlags(m) == QUIET)
            moves[quietCount++] = moves[i];
    }

    return quietCount;
}