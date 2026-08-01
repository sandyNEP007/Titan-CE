#include "iterativeDeepening.h"
#include "search.h"
Move IterativeDeepening::search(Board& board,
                                  bool maximizingPlayer,
                                  int maxDepth)
{
    Search engine(board);

    Move bestMove;

    for (int depth = 1; depth <= maxDepth; depth++)
    {
            std::vector<Move> legalMoves =
            board.generateLegalMoves(maximizingPlayer);

            int bestScore =
            maximizingPlayer ? -1000000 : 1000000;

        for (Move move : legalMoves)
        {
            board.makeMove(move);

            int score =
                engine.minimax( depth - 1,
                               -1000000,
                               1000000,
                               !maximizingPlayer);

            board.undoMove(move);

            if (maximizingPlayer)
            {
                if (score > bestScore)
                {
                    bestScore = score;
                    bestMove = move;
                }
            }
            else
            {
                if (score < bestScore)
                {
                    bestScore = score;
                    bestMove = move;
                }
            }
        }

       
    }

    return bestMove;
}