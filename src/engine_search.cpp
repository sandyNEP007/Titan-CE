#include "engine_search.h"
#include "movegen.h"
#include "evaluation.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <chrono>

int EngineSearch::historyTable[2][64][64] = {};
Move EngineSearch::killerMoves[EngineSearch::MAX_PLY][2];
std::unordered_map<uint64_t, EngineSearch::TTEntry> EngineSearch::transpositionTable;
long long EngineSearch::nodes = 0;
long long EngineSearch::cutoffs = 0;
long long EngineSearch::qNodes = 0;
long long EngineSearch::qCutoffs = 0;
long long EngineSearch::ttHits = 0;
std::chrono::steady_clock::time_point EngineSearch::searchDeadline{};
bool EngineSearch::stopSearch = false;

bool EngineSearch::checkTimeUp()
{
    // searchDeadline default-constructed (epoch) means "no time limit".
    if (searchDeadline.time_since_epoch().count() == 0)
        return false;

    return std::chrono::steady_clock::now() >= searchDeadline;
}

//search the moves
int EngineSearch::negamax(Board& board, int alpha, int beta, int depth, int ply)
{
    if (stopSearch)
        return 0;

    nodes++;

    // Check the clock periodically, not every node (syscall cost).
    if ((nodes & 2047) == 0 && checkTimeUp())
    {
        stopSearch = true;
        return 0;
    }

    uint64_t hash = board.getZobristHash();
    int originalAlpha = alpha;
    Move hashMove;
    bool hasHashMove = false;

    auto ttIt = transpositionTable.find(hash);
    if (ttIt != transpositionTable.end())
    {
        const TTEntry& entry = ttIt->second;

        if (entry.depth >= depth)
        {
            ttHits++;
            hashMove = entry.bestMove;
            hasHashMove = true;

            int ttScore = scoreFromTT(entry.score, ply);

            if (entry.flag == TT_EXACT)
                return ttScore;

            if (entry.flag == TT_ALPHA && ttScore <= alpha)
                return alpha;

            if (entry.flag == TT_BETA && ttScore >= beta)
                return beta;
        }
    }

    if (depth == 0)
        return quiescence(board, alpha, beta, 0);

    bool inCheck = board.isKingInCheck(board.isWhiteTurn());

    // Null-move pruning
    if (!inCheck &&
        depth >= 3 &&
        beta < MATE_THRESHOLD &&
        alpha > -MATE_THRESHOLD &&
        hasNonPawnMaterial(board))
    {
        int reduction = (depth > 6) ? 3 : 2;

        board.makeNullMove();
        int nullScore = -negamax(board, -beta, -beta + 1, depth - 1 - reduction, ply + 1);
        board.undoNullMove();

        if (stopSearch)
            return 0;

        if (nullScore >= beta)
        {
            cutoffs++;
            return beta;
        }
    }

    std::vector<Move> legalMoves = MoveGenerator::generateLegalMoves(board);

    std::stable_sort(
        legalMoves.begin(),
        legalMoves.end(),
        [&](const Move& a, const Move& b)
        {
            int scoreA = getMoveOrderingScore(board, a, depth);
            int scoreB = getMoveOrderingScore(board, b, depth);

            if (hasHashMove && a == hashMove) scoreA += 1000000;
            if (hasHashMove && b == hashMove) scoreB += 1000000;

            return scoreA > scoreB;
        }
    );

    if (legalMoves.empty())
    {
        if (inCheck)
            return -(MATE_VALUE - ply);

        return 0;
    }

    int bestScore = -1000000;
    Move bestMove;
    int moveIndex = 0;

    for (const Move& move : legalMoves)
    {
        bool isQuiet = !isCapture(board, move);   // must check BEFORE making the move

        board.makeMove(move);

        bool givesCheckFlag = board.isKingInCheck(board.isWhiteTurn());

        int score;

        if (moveIndex == 0)
        {
            score = -negamax(board, -beta, -alpha, depth - 1, ply + 1);
        }
        else
        {
            // Late Move Reduction: reduce depth for late, quiet, non-tactical moves.
            int reduction = 0;

            if (moveIndex >= LMR_MIN_MOVE_INDEX &&
                depth >= LMR_MIN_DEPTH &&
                isQuiet &&
                !inCheck &&
                !givesCheckFlag &&
                !(hasHashMove && move == hashMove) &&
                !(move == killerMoves[depth][0]) &&
                !(move == killerMoves[depth][1]))
            {
                reduction = (moveIndex >= LMR_MIN_MOVE_INDEX + 6 && depth >= 6) ? 2 : 1;
            }

            int reducedDepth = depth - 1 - reduction;
            if (reducedDepth < 0) reducedDepth = 0;

            // Cheap null-window probe, possibly reduced.
            score = -negamax(board, -alpha - 1, -alpha, reducedDepth, ply + 1);

            // Reduced probe beat alpha -> re-verify at full depth, still null-window.
            if (!stopSearch && score > alpha && reduction > 0)
            {
                score = -negamax(board, -alpha - 1, -alpha, depth - 1, ply + 1);
            }

            // Still looks better than alpha -> full window re-search for accurate score.
            if (!stopSearch && score > alpha && score < beta)
            {
                score = -negamax(board, -beta, -alpha, depth - 1, ply + 1);
            }
        }

        board.undoMove();
        moveIndex++;

        if (stopSearch)
            return 0;

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha)
            alpha = score;

        if (alpha >= beta)
        {
            cutoffs++;

            if (isQuiet)
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
    if (bestScore <= originalAlpha)      flag = TT_ALPHA;
    else if (bestScore >= beta)          flag = TT_BETA;
    else                                  flag = TT_EXACT;

    TTEntry entry;
    entry.hash = hash;
    entry.depth = depth;
    entry.score = scoreToTT(bestScore, ply);
    entry.flag = flag;
    entry.bestMove = bestMove;
    transpositionTable[hash] = entry;

    return bestScore;
}

int EngineSearch::quiescence(Board& board, int alpha, int beta, int checkPly)
{
    qNodes++;

    if ((qNodes & 4095) == 0 && checkTimeUp())
    {
        stopSearch = true;
        return 0;
    }

    int evaluation = Evaluation::boardEvaluation(board);
    int standPat = board.isWhiteTurn() ? evaluation : -evaluation;

    bool inCheck = board.isKingInCheck(board.isWhiteTurn());

    if (!inCheck)
    {
        if (standPat >= beta)
        {
            qCutoffs++;
            return beta;
        }
        if (standPat > alpha)
            alpha = standPat;
    }

    std::vector<Move> legalMoves = MoveGenerator::generateLegalMoves(board);
    std::vector<Move> tacticalMoves;

    for (const Move& move : legalMoves)
    {
        bool capture = isCapture(board, move);

        if (inCheck)
        {
            tacticalMoves.push_back(move);
            continue;
        }

        if (capture)
        {
            tacticalMoves.push_back(move);
            continue;
        }

        if (checkPly < MAX_QUIESCENCE_CHECK_PLY && givesCheck(board, move))
        {
            tacticalMoves.push_back(move);
        }
    }

    std::stable_sort(
        tacticalMoves.begin(),
        tacticalMoves.end(),
        [&](const Move& a, const Move& b)
        {
            return getMVVLVAScore(board, a) > getMVVLVAScore(board, b);
        }
    );

    int movesSearched = 0;

    for (const Move& move : tacticalMoves)
    {
        bool wasCapture = isCapture(board, move);

        board.makeMove(move);

        int nextCheckPly = wasCapture ? checkPly : checkPly + 1;
        int score = -quiescence(board, -beta, -alpha, nextCheckPly);

        board.undoMove();
        movesSearched++;

        if (stopSearch)
            return 0;

        if (score >= beta)
        {
            qCutoffs++;
            return beta;
        }
        if (score > alpha)
            alpha = score;
    }

    if (inCheck && movesSearched == 0)
        return -(MATE_VALUE - checkPly);

    return alpha;
}

Move EngineSearch::findBestMove(Board& board, int maxdepth, long long timeLimitMs)
{
    auto searchStart = std::chrono::steady_clock::now();

    stopSearch = false;

    if (timeLimitMs > 0)
        searchDeadline = searchStart + std::chrono::milliseconds(timeLimitMs);
    else
        searchDeadline = std::chrono::steady_clock::time_point{};

    nodes = 0;
    cutoffs = 0;
    qNodes = 0;
    qCutoffs = 0;
    ttHits = 0;
    transpositionTable.clear();

    for (int side = 0; side < 2; side++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                historyTable[side][from][to] = 0;

    for (int ply = 0; ply < MAX_PLY; ply++)
    {
        killerMoves[ply][0] = Move();
        killerMoves[ply][1] = Move();
    }

    Move bestMove;
    Move previousBestMove;
    int previousScore = 0;
    const int ASPIRATION_WINDOW = 50;
    const int ASPIRATION_MAX = 1000000;

    for (int depth = 1; depth <= maxdepth; depth++)
    {
        std::vector<Move> legalMoves = MoveGenerator::generateLegalMoves(board);

        std::stable_sort(
            legalMoves.begin(),
            legalMoves.end(),
            [&](const Move& a, const Move& b)
            {
                int scoreA = getMoveOrderingScore(board, a, depth);
                int scoreB = getMoveOrderingScore(board, b, depth);

                if (a == previousBestMove) scoreA += 100000;
                if (b == previousBestMove) scoreB += 100000;

                return scoreA > scoreB;
            }
        );

        int currentBestScore = -1000000;
        Move currentBestMove;

        int alpha, beta;
        int window = ASPIRATION_WINDOW;

        if (depth == 1)
        {
            alpha = -ASPIRATION_MAX;
            beta = ASPIRATION_MAX;
        }
        else
        {
            alpha = previousScore - window;
            beta = previousScore + window;
        }

        bool depthAborted = false;

        while (true)
        {
            currentBestScore = -1000000;

            int searchAlpha = alpha;
            int searchBeta = beta;
            int moveIndex = 0;

            for (const Move& move : legalMoves)
            {
                board.makeMove(move);

                int score;

                if (moveIndex == 0)
                {
                    score = -negamax(board, -searchBeta, -searchAlpha, depth - 1, 1);
                }
                else
                {
                    score = -negamax(board, -searchAlpha - 1, -searchAlpha, depth - 1, 1);

                    if (!stopSearch && score > searchAlpha && score < searchBeta)
                    {
                        score = -negamax(board, -searchBeta, -searchAlpha, depth - 1, 1);
                    }
                }

                board.undoMove();
                moveIndex++;

                if (stopSearch)
                {
                    depthAborted = true;
                    break;
                }

                if (score > currentBestScore)
                {
                    currentBestScore = score;
                    currentBestMove = move;
                }

                if (score > searchAlpha)
                    searchAlpha = score;

                // Fail-high: window too narrow, no point finishing this pass.
                if (searchAlpha >= searchBeta)
                    break;
            }

            if (depthAborted)
                break;

            if (currentBestScore > alpha && currentBestScore < beta)
                break;

            window *= 4;
            if (window > ASPIRATION_MAX) window = ASPIRATION_MAX;

            if (currentBestScore <= alpha)
                alpha = std::max(-ASPIRATION_MAX, previousScore - window);

            if (currentBestScore >= beta)
                beta = std::min(ASPIRATION_MAX, previousScore + window);
        }

        if (depthAborted)
            break;  // keep bestMove from the last fully-completed depth

        bestMove = currentBestMove;
        previousBestMove = bestMove;
        previousScore = currentBestScore;

        auto now = std::chrono::steady_clock::now();
        long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStart).count();
        long long totalNodes = nodes + qNodes;
        long long nps = (elapsedMs > 0) ? (totalNodes * 1000 / elapsedMs) : totalNodes;

        char fromFile = 'a' + bestMove.fromCol;
        char fromRank = '8' - bestMove.fromRow;
        char toFile   = 'a' + bestMove.toCol;
        char toRank   = '8' - bestMove.toRow;

        std::cout << "info depth " << depth
                   << " score cp " << currentBestScore
                   << " nodes " << totalNodes
                   << " nps " << nps
                   << " time " << elapsedMs
                   << " pv " << fromFile << fromRank << toFile << toRank
                   << "\n";
        std::cout.flush();
    }

    return bestMove;
}

int EngineSearch::getMVVLVAScore(Board& board, const Move& move)
{
    char attacker = board.getPiece(move.fromRow, move.fromCol);
    char victim = board.getPiece(move.toRow, move.toCol);

    if (victim == '.')
        return 0;

    auto pieceValue = [](char piece) -> int
    {
        switch (piece)
        {
            case 'P': case 'p': return 100;
            case 'N': case 'n': return 320;
            case 'B': case 'b': return 330;
            case 'R': case 'r': return 500;
            case 'Q': case 'q': return 900;
            case 'K': case 'k': return 20000;
        }
        return 0;
    };

    return pieceValue(victim) * 10 - pieceValue(attacker);
}

int EngineSearch::getMoveOrderingScore(Board& board, const Move& move, int depth)
{
    int score = 0;

    score += getMVVLVAScore(board, move);

    if (depth < MAX_PLY)
    {
        if (move == killerMoves[depth][0]) score += 9000;
        else if (move == killerMoves[depth][1]) score += 8000;
    }

    if (!isCapture(board, move))
    {
        int side = board.isWhiteTurn() ? 0 : 1;
        int fromSquare = move.fromRow * 8 + move.fromCol;
        int toSquare = move.toRow * 8 + move.toCol;
        score += historyTable[side][fromSquare][toSquare];
    }

    return score;
}

bool EngineSearch::isCapture(Board& board, const Move& move)
{
    return board.getPiece(move.toRow, move.toCol) != '.';
}

int EngineSearch::scoreToTT(int score, int ply)
{
    if (score >= MATE_THRESHOLD)  return score + ply;
    if (score <= -MATE_THRESHOLD) return score - ply;
    return score;
}

int EngineSearch::scoreFromTT(int score, int ply)
{
    if (score >= MATE_THRESHOLD)  return score - ply;
    if (score <= -MATE_THRESHOLD) return score + ply;
    return score;
}

bool EngineSearch::givesCheck(Board& board, const Move& move)
{
    board.makeMove(move);
    bool check = board.isKingInCheck(board.isWhiteTurn());
    board.undoMove();
    return check;
}

bool EngineSearch::hasNonPawnMaterial(Board& board)
{
    bool white = board.isWhiteTurn();

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);
            if (piece == '.') continue;

            if (board.isWhitePiece(piece) != white) continue;

            char upper = toupper(piece);
            if (upper == 'N' || upper == 'B' || upper == 'R' || upper == 'Q')
                return true;
        }
    }

    return false;
}