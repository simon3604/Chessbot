#include "movePicker.h"
#include "makeMove.h"
#include "misc.h"

void MovePicker::scoreMoves(int start, int end)
{

  for (int i = start; i < end; i++)
  {
    Move m = moves[i];
    scores[i] = 0;
    Piece victim = m.captured;
    Piece attacker = getPieceType(board, m.from);

    // 1. TT move
    if (m.from == ttMove.from && m.to == ttMove.to)
      scores[i] += 1000000;

    // 2. Promotions
    if (m.promotion != NONE)
      scores[i] += 100000 + pieceValue(m.promotion);

    // 3. Captures
    if (victim != NONE)
      scores[i] += 10000 + 10 * pieceValue(victim) - pieceValue(attacker);

    // 4. Killer moves
    if (m == killerMoves[ply][0])
      scores[i] += 9000;
    if (m == killerMoves[ply][1])
      scores[i] += 8000;

    // 5. History
    scores[i] += history[board.sideToMove][m.from][m.to];
  }
}

Move MovePicker::pickBest()
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

    Move m = moves[index++];
    if (m.piece == NONE)
    {
      std::cout << "CORRUPTED MOVE DETECTED\n";
      std::cout << "from=" << m.from << " to=" << m.to << "\n";
      exit(1);
    }
    if (m == ttMove)
      continue;

    return m;
  }

  return Move{-1, -1, -1, -1, NONE, NONE};
}

Move MovePicker::nextMove()
{

  while (true)
  {
    // === 1. TT MOVE ===
    if (stage == TT_STAGE)
    {
      stage = CAPTURE_STAGE;

      if (ttMove.from != -1)
      {
        Color originalSide = board.sideToMove;
        Undo u = makeMove(ttMove, board);

        if (!isKingInCheck(originalSide, board))
        {
          undoMove(ttMove, board, u);
          return ttMove;
        }

        undoMove(ttMove, board, u);
      }
    }

    // === 2. CAPTURES ===
    if (stage == CAPTURE_STAGE)
    {

      if (moveCount == 0)
      {
        moveCount = generateCaptures(board, board.sideToMove, moves);

        captureEnd = moveCount;
        scoreMoves(0, captureEnd);

        index = 0;
      }

      Move m;
      while ((m = pickBest()).from != -1)
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
        Move m = killerMoves[ply][killerIndex++];

        if (m == ttMove)
          continue;

        if (m.piece == NONE)
          continue;

        if (getPieceType(board, m.from) != m.piece)
          continue;

        Color originalSide = board.sideToMove;
        Undo u = makeMove(m, board);

        if (!isKingInCheck(originalSide, board))
        {
          undoMove(m, board, u);
          return m;
        }

        undoMove(m, board, u);
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
        int count = generateLegalMoves(board, board.sideToMove, moves + captureEnd);

        int quietCount = 0;

        for (int i = 0; i < count; i++)
        {
          Move m = moves[captureEnd + i];

          if (m.captured == NONE && m.promotion == NONE)
          {
            moves[captureEnd + quietCount++] = m;
          }
        }

        int quietStart = captureEnd;
        moveCount = captureEnd + quietCount;

        scoreMoves(quietStart, moveCount);

        index = captureEnd;
      }

      Move m;
      while ((m = pickBest()).from != -1)
      {
        return m;
      }

      stage = DONE_STAGE;
    }

    // === DONE ===
    return Move{-1, -1, -1, -1, NONE, NONE};
  }
}

MovePicker::MovePicker(Board &b, Move tt, int pl)
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