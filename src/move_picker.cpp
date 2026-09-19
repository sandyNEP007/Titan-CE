#include "move_picker.h"
#include "engine_search.h"

MovePicker::MovePicker(
    Board& board,
    std::vector<Move>& moves,
    const Move& ttMove,
    int depth
)
    : board(board),
      depth(depth),
      ttMove(ttMove),
      goodCaptureIndex(0),
      goodQuietIndex(0),
      badCaptureIndex(0),
      badQuietIndex(0),
      stage(MAIN_TT),
      ttMoveUsed(false)
{
    // A TT move is only trustworthy if it's actually a legal move in
    // THIS position — hash collisions can hand back a stale move from
    // a different position sharing the same table slot.
    ttMoveValid = false;

    if (ttMove.fromRow != -1)
    {
        for (const Move& m : moves)
        {
            if (m == ttMove)
            {
                ttMoveValid = true;
                break;
            }
        }
    }

    for (const Move& move : moves)
    {
        // Don't add the TT move here — it's dispensed separately in the
        // MAIN_TT stage, and would otherwise be searched twice.
        if (ttMoveValid && move == ttMove)
            continue;

        if (board.getPiece(move.toRow, move.toCol) != '.')
        {
            captures.push_back(move);
        }
        else
        {
            quiets.push_back(move);
        }
    }
}

Move MovePicker::nextMove()
{
    while (true)
    {
        switch (stage)
        {
            case MAIN_TT:
            {
                stage = CAPTURE_INIT;

                if (ttMoveValid && !ttMoveUsed)
                {
                    ttMoveUsed = true;
                    return ttMove;
                }

                break;
            }

            case CAPTURE_INIT:
            {
                for (const Move& move : captures)
                {
                    int seeScore = EngineSearch::see(board, move);

                    if (seeScore >= 0)
                        goodCaptures.push_back(move);
                    else
                        badCaptures.push_back(move);
                }

                auto seeCompare = [&](const Move& a, const Move& b)
                {
                    return EngineSearch::see(board, a) > EngineSearch::see(board, b);
                };

                std::sort(goodCaptures.begin(), goodCaptures.end(), seeCompare);
                std::sort(badCaptures.begin(), badCaptures.end(), seeCompare);

                stage = GOOD_CAPTURE;
                break;
            }

            case GOOD_CAPTURE:
            {
                if (goodCaptureIndex < (int)goodCaptures.size())
                    return goodCaptures[goodCaptureIndex++];

                stage = QUIET_INIT;
                break;
            }

            case QUIET_INIT:
            {
                int side = board.isWhiteTurn() ? 0 : 1;

                auto quietScore = [&](const Move& move) -> int
                {
                    int score = 0;

                    if (depth < EngineSearch::MAX_PLY)
                    {
                        if (move == EngineSearch::killerMoves[depth][0])
                            score += 9000;
                        else if (move == EngineSearch::killerMoves[depth][1])
                            score += 8000;
                    }

                    int fromSquare = move.fromRow * 8 + move.fromCol;
                    int toSquare = move.toRow * 8 + move.toCol;

                    score += EngineSearch::historyTable[side][fromSquare][toSquare];

                    return score;
                };

                for (const Move& move : quiets)
                {
                    if (quietScore(move) > 0)
                        goodQuiets.push_back(move);
                    else
                        badQuiets.push_back(move);
                }

                auto quietCompare = [&](const Move& a, const Move& b)
                {
                    return quietScore(a) > quietScore(b);
                };

                std::sort(goodQuiets.begin(), goodQuiets.end(), quietCompare);
                std::sort(badQuiets.begin(), badQuiets.end(), quietCompare);

                stage = GOOD_QUIET;
                break;
            }

            case GOOD_QUIET:
            {
                if (goodQuietIndex < (int)goodQuiets.size())
                    return goodQuiets[goodQuietIndex++];

                stage = BAD_CAPTURE;
                break;
            }

            case BAD_CAPTURE:
            {
                if (badCaptureIndex < (int)badCaptures.size())
                    return badCaptures[badCaptureIndex++];

                stage = BAD_QUIET;
                break;
            }

            case BAD_QUIET:
            {
                if (badQuietIndex < (int)badQuiets.size())
                    return badQuiets[badQuietIndex++];

                stage = DONE;
                break;
            }

            case DONE:
            default:
                return Move(); 
        }
    }
}