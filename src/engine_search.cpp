#include "engine_search.h"
#include "movegen.h"
#include "evaluation.h"
#include <algorithm>
#include <limits>
#include <iostream>

int EngineSearch::historyTable[2][64][64] = {};
Move EngineSearch::killerMoves[EngineSearch::MAX_PLY][2];
std::unordered_map<uint64_t, EngineSearch::TTEntry>EngineSearch::transpositionTable;
long long EngineSearch::nodes = 0;
long long EngineSearch::cutoffs = 0;
long long EngineSearch::qNodes = 0;
long long EngineSearch::qCutoffs = 0;
long long EngineSearch::ttHits = 0;

//search the moves
int EngineSearch::negamax(Board& board, int alpha, int beta, int depth)
{
    nodes++;
    uint64_t hash = board.getZobristHash();
    int originalAlpha = alpha;
    Move hashMove;
    bool hasHashMove = false;

//transposition
auto ttIt =transpositionTable.find(hash);
if (ttIt != transpositionTable.end())
{
    const TTEntry& entry = ttIt->second;

    if (entry.depth >= depth)
    {
        ttHits++;
        hashMove = entry.bestMove;
        hasHashMove = true;

        if (entry.flag == TT_EXACT)
        {
            return entry.score;
        }

        if (entry.flag == TT_ALPHA &&
            entry.score <= alpha)
        {
            return alpha;
        }

        if (entry.flag == TT_BETA &&
            entry.score >= beta)
        {
            return beta;
        }
    }
}

    // Leaf node
    if (depth == 0)
    {
        return quiescence(board, alpha, beta);  
    }

    // Generate all legal moves
    std::vector<Move> legalMoves = MoveGenerator::generateLegalMoves(board);

    std::stable_sort(
    legalMoves.begin(),
    legalMoves.end(),
    [&](const Move& a, const Move& b)
    {
        int scoreA = getMoveOrderingScore(board, a, depth);
        int scoreB = getMoveOrderingScore(board, b, depth);
        
        if (hasHashMove && a == hashMove)
            scoreA += 1000000;

        if (hasHashMove && b == hashMove)
            scoreB += 1000000;

        return scoreA > scoreB;
    }
);

    // No legal moves
    if (legalMoves.empty())
    {
        bool sideToMove = board.isWhiteTurn();

        // Current player is in check → checkmate
        if (board.isKingInCheck(sideToMove))
        {
            return -100000;
        }

        // Current player is not in check → stalemate
        return 0;
    }

    int bestScore = -1000000;
    Move bestMove;

    for (const Move& move : legalMoves)
{
        board.makeMove(move);

        int score = -negamax(board, -beta, -alpha, depth - 1);

        board.undoMove();

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        //update alpha
        if (score > alpha)
        {
            alpha = score;
        }

       if (alpha >= beta)
     {
       cutoffs++;

        if (!isCapture(board, move))
     {

        if (depth < MAX_PLY)
        {
            if (!(move == killerMoves[depth][0]))
            {
                killerMoves[depth][1] = killerMoves[depth][0];
                killerMoves[depth][0] = move;
            }
        }

         int side = board.isWhiteTurn() ? 0 : 1;
         int fromSquare = move.fromRow * 8 + move.fromCol;
         int toSquare = move.toRow * 8 + move.toCol;

        historyTable[side][fromSquare][toSquare] += depth * depth;
    }

    break;
  }


}

TTFlag flag;

if (bestScore <= originalAlpha)
{
    flag = TT_ALPHA;
}
else if (bestScore >= beta)
{
    flag = TT_BETA;
}
else
{
    flag = TT_EXACT;
}

//storing tt-entry
TTEntry entry;
entry.hash = hash;
entry.depth = depth;
entry.score = bestScore;
entry.flag = flag;
entry.bestMove = bestMove;
transpositionTable[hash] = entry;

    return bestScore;

}


int EngineSearch::quiescence(
    Board& board,
    int alpha,
    int beta)
{
    qNodes++;

    int evaluation =
        Evaluation::boardEvaluation(board);

    int standPat;

    if (board.isWhiteTurn())
        standPat = evaluation;
    else
        standPat = -evaluation;

    // -------------------------------------------------
    // STAND-PAT BETA CUTOFF
    // -------------------------------------------------

    if (standPat >= beta)
    {
        qCutoffs++;
        return beta;
    }

    if (standPat > alpha)
        alpha = standPat;

    // -------------------------------------------------
    // GENERATE LEGAL MOVES
    // -------------------------------------------------

    std::vector<Move> legalMoves =
        MoveGenerator::generateLegalMoves(board);

    // -------------------------------------------------
    // KEEP ONLY CAPTURES
    // -------------------------------------------------

    std::vector<Move> captures;

    for (const Move& move : legalMoves)
    {
        if (isCapture(board, move))
        {
            captures.push_back(move);
        }
    }

    // -------------------------------------------------
    // MVV-LVA ORDERING
    // -------------------------------------------------

    std::stable_sort(
        captures.begin(),
        captures.end(),
        [&](const Move& a, const Move& b)
        {
            return getMVVLVAScore(board, a) >
                   getMVVLVAScore(board, b);
        }
    );

    // -------------------------------------------------
    // SEARCH CAPTURES
    // -------------------------------------------------

    for (const Move& move : captures)
    {
        board.makeMove(move);

        int score =
            -quiescence(
                board,
                -beta,
                -alpha);

        board.undoMove();

        // Beta cutoff
        if (score >= beta)
        {
            qCutoffs++;
            return beta;
        }

        if (score > alpha)
            alpha = score;
    }

    return alpha;
}
//find best move after search

Move EngineSearch::findBestMove(Board& board, int maxdepth)
{

nodes = 0;
cutoffs = 0;
qNodes = 0;
qCutoffs = 0;
ttHits = 0;
transpositionTable.clear();

//history heuristic
for (int side = 0; side < 2; side++)
{
    for (int from = 0; from < 64; from++)
    {
        for (int to = 0; to < 64; to++)
        {
            historyTable[side][from][to] = 0;
        }
    }
}

//killer moves
for (int ply = 0; ply < MAX_PLY; ply++)
{
    killerMoves[ply][0] = Move();
    killerMoves[ply][1] = Move();
}

Move bestMove;
Move previousBestMove;
int previousScore = 0;
const int ASPIRATION_WINDOW = 50;

//iterative deepening
for (int depth = 1; depth <= maxdepth; depth++)
{
    std::vector<Move> legalMoves =
        MoveGenerator::generateLegalMoves(board);

    std::stable_sort(
        legalMoves.begin(),
        legalMoves.end(),
        [&](const Move& a, const Move& b)
        {
            int scoreA =
                getMoveOrderingScore(board, a, depth);

            int scoreB =
                getMoveOrderingScore(board, b, depth);

            if (a == previousBestMove)
                scoreA += 100000;

            if (b == previousBestMove)
                scoreB += 100000;

            return scoreA > scoreB;
        }
    );

    int currentBestScore = -1000000;
    Move currentBestMove;

    int alpha;
    int beta;

    // First iteration: full window
    if (depth == 1)
    {
        alpha = -1000000;
        beta = 1000000;
    }
    else
    {
        alpha = previousScore - ASPIRATION_WINDOW;
        beta = previousScore + ASPIRATION_WINDOW;
    }

    while (true)
    {
        currentBestScore = -1000000;

        int searchAlpha = alpha;
        int searchBeta = beta;

        for (const Move& move : legalMoves)
        {
            board.makeMove(move);

            int score =
                -negamax(
                    board,
                    -searchBeta,
                    -searchAlpha,
                    depth - 1);

            board.undoMove();

            if (score > currentBestScore)
            {
                currentBestScore = score;
                currentBestMove = move;
            }

            if (score > searchAlpha)
                searchAlpha = score;
        }

        // Score fits inside the aspiration window.
        if (currentBestScore > alpha &&
            currentBestScore < beta)
        {
            break;
        }

        // Fail-low → widen downward.
        if (currentBestScore <= alpha)
        {
            alpha = -1000000;
        }

        // Fail-high → widen upward.
        if (currentBestScore >= beta)
        {
            beta = 1000000;
        }
    }

    bestMove = currentBestMove;
    previousBestMove = bestMove;

    previousScore = currentBestScore;
}

return bestMove;

}

int EngineSearch::getMVVLVAScore(
    Board& board,
    const Move& move)
{
    char attacker =
        board.getPiece(move.fromRow, move.fromCol);

    char victim =
        board.getPiece(move.toRow, move.toCol);

    // Not a capture
    if (victim == '.')
        return 0;

    auto pieceValue = [](char piece) -> int
    {
        switch (piece)
        {
            case 'P':
            case 'p':
                return 100;

            case 'N':
            case 'n':
                return 320;

            case 'B':
            case 'b':
                return 330;

            case 'R':
            case 'r':
                return 500;

            case 'Q':
            case 'q':
                return 900;

            case 'K':
            case 'k':
                return 20000;
        }

        return 0;
    };

    int victimValue = pieceValue(victim);
    int attackerValue = pieceValue(attacker);

    return victimValue * 10 - attackerValue;
}

int EngineSearch::getMoveOrderingScore(
    Board& board,
    const Move& move,
    int depth)
{
    int score = 0;

    // Captures first
    score += getMVVLVAScore(board, move);

    // Killer moves
    if (depth < MAX_PLY)
    {
        if (move == killerMoves[depth][0])
            score += 9000;

        else if (move == killerMoves[depth][1])
            score += 8000;
    }

    //history heuristic
    if (!isCapture(board, move))
    {
        int side = board.isWhiteTurn() ? 0 : 1;
        int fromSquare = move.fromRow * 8 + move.fromCol;
        int toSquare = move.toRow * 8 + move.toCol;

        score += historyTable[side][fromSquare][toSquare];

    }
    return score;
}

bool EngineSearch::isCapture(
    Board& board,
    const Move& move)
{
    return board.getPiece(
        move.toRow,
        move.toCol) != '.';
}