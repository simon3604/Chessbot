#include "movePicker.h"
#include "makeMove.h"
#include "misc.h"
#include "attack.h"

using u32 = uint32_t;

void MovePicker::scoreMoves(int start, int end)
{

  for (int i = start; i < end; i++)
  {
    u32 m = moves[i];
    scores[i] = 0;
    Piece victim = (moveFlags(m) & ENPASSANT) ? getPieceType(board, (board.sideToMove == WHITE) ? toSquare(m) - 8 : toSquare(m) + 8) : getPieceType(board, fromSquare(m));

    Piece attacker = getPieceType(board, fromSquare(m));

    // 1. TT move
    if (ttMove != 0ULL && fromSquare(m) == fromSquare(ttMove) && toSquare(m) == toSquare(ttMove))
      scores[i] += 1000000;

    // 2. Promotions
    if (promotionPiece(m) != NONE)
      scores[i] += 100000 + pieceValue(promotionPiece(m));

    // 3. Captures
    if (victim != NONE)
      scores[i] += 10000 + 10 * pieceValue(victim) - pieceValue(attacker);

    // 4. Killer moves
    if (m == killerMoves[ply][0])
      scores[i] += 9000;
    if (m == killerMoves[ply][1])
      scores[i] += 8000;

    // 5. History
    scores[i] += history[board.sideToMove][fromSquare(m)][toSquare(m)];
  }
}

u32 MovePicker::pickBest()
{
  int end = (stage == CAPTURE_STAGE) ? captureEnd : moveCount;

  while (index < end)
  {
    int best = index;
    for (int i = index + 1; i < end; i++)
    {
      if (scores[i] > scores[best])
        best = i;
    }

    std::swap(moves[index], moves[best]);
    std::swap(scores[index], scores[best]);

    u32 m = moves[index++];
    if (getPieceType(board, fromSquare(m)) == NONE)
    {
      std::cerr << "CORRUPTED MOVE DETECTED\n";
      std::cerr << "from=" << fromSquare(m) << " to=" << toSquare(m) << "\n";
      exit(1);
    }
    if (m == ttMove)
      continue;

    return m;
  }

  return 0;
}

u32 MovePicker::nextMove()
{

  while (true)
  {
    // === 1. TT MOVE ===
    if (stage == TT_STAGE)
    {
      stage = CAPTURE_STAGE;

      Color side = pieceColor(getPieceType(board, fromSquare(ttMove)), "NONE");
      if ((getPieceType(board, fromSquare(ttMove)) == NONE && ttMove != 0ULL))
      {
        ttMove = 0;
      }
      if (toSquare(ttMove) & (side == WHITE) ? board.all_white : board.all_black) {
         ttMove = 0;
      }
      if (ttMove != 0ULL)
      {

        Color originalSide = board.sideToMove;
        Undo u = makeMove(ttMove, board, "MovePicker, TT");

        if (!isKingInCheck(originalSide, board, "MovePicker, TT"))
        {
          undoMove(ttMove, board, u, "MovePicker, TT");
          return ttMove;
        }

        undoMove(ttMove, board, u, "MovePicker, TT 2");
      }
    }

    // === 2. CAPTURES ===
    if (stage == CAPTURE_STAGE)
    {

      if (moveCount == 0)
      {
        moveCount = generateCaptures(board, board.sideToMove, moves, "MovePicker, Captures");

        captureEnd = moveCount;
        scoreMoves(0, captureEnd);

        index = 0;
      }

      u32 m;
      while ((m = pickBest()) != 0)
      {
        return m;
      }

      stage = KILLER_STAGE;
    }

    // === 3. KILLERS ===
    if (stage == KILLER_STAGE)
    {
      while (killerIndex < 2)
      {
        u32 m = killerMoves[ply][killerIndex++];

        if (m == ttMove)
          continue;

        if (!isMoveLegal(board, m, "MovePicker, Killers"))
          continue;

        bool existsInMoves = false;
        for (int i = 0; i < MAX_MOVES; i++)
        {

          if (moves[i] == m)
          {
            existsInMoves = true;
            break;
          }
        }

        Color originalSide = board.sideToMove;

        Undo u = makeMove(m, board, "MovePicker, Killers");

        if (!isKingInCheck(originalSide, board, "MovePicker, Killers"))
        {
          undoMove(m, board, u, "MovePicker, Killers");
          
          return m;
        }

        undoMove(m, board, u, "MovePicker, Killers 2");
      }

      stage = QUIET_STAGE;
      index = captureEnd;
      killerIndex = 0;
    }

    // === 4. QUIETS ===
    if (stage == QUIET_STAGE)
    {

      if (moveCount == captureEnd)
      {
        int quietCount = generateQuietMoves(board, board.sideToMove, moves + captureEnd);

        int quietStart = captureEnd;
        moveCount = captureEnd + quietCount;

        scoreMoves(quietStart, moveCount);

        index = captureEnd;
      }

      u32 m;
      while ((m = pickBest()) != 0)
      {
        
        return m;
      }

      stage = DONE_STAGE;
    }

    // === DONE ===
    return 0;
  }
}

MovePicker::MovePicker(Board &b, u32 tt, int pl)
    : board(b), ttMove(tt), ply(pl),
      stage(TT_STAGE),
      index(0),
      moveCount(0),
      killerIndex(0),
      captureEnd(0)
{
  moveCount = 0;
  stage = TT_STAGE;
}