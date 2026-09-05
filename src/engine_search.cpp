#include "engine_search.h"
#include "movegen.h"
#include "evaluation.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <chrono>
#include <cctype>

int EngineSearch::historyTable[2][64][64] = {};
Move EngineSearch::killerMoves[EngineSearch::MAX_PLY][2];
EngineSearch::TTEntry EngineSearch::transpositionTable[EngineSearch::TT_SIZE] = {};
long long EngineSearch::nodes = 0;
long long EngineSearch::cutoffs = 0;
long long EngineSearch::qNodes = 0;
long long EngineSearch::qCutoffs = 0;
long long EngineSearch::ttHits = 0;
std::chrono::steady_clock::time_point EngineSearch::searchDeadline{};
bool EngineSearch::stopSearch = false;

bool EngineSearch::checkTimeUp()
{
    if (searchDeadline.time_since_epoch().count() == 0)
        return false;

    return std::chrono::steady_clock::now() >= searchDeadline;
}

bool EngineSearch::hasTimeForNextDepth(
    std::chrono::steady_clock::time_point searchStart,
    long long timeLimitMs,
    long long lastDepthMs)
{
    if (timeLimitMs <= 0)
        return true;

    auto now = std::chrono::steady_clock::now();
    long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - searchStart
    ).count();

    long long remainingMs = timeLimitMs - elapsedMs;
    long long predictedNextDepthMs = lastDepthMs * 4;

    return predictedNextDepthMs < remainingMs;
}

// ---- Static Exchange Evaluation helpers ----
// File-local helper: finds the cheapest attacker of `white`'s color that
// can capture on (targetRow, targetCol) within the given occupancy grid.
// Not a class member — only ever used inside EngineSearch::see.
static bool seeSquareOnBoard(int r, int c)
{
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

static bool seeFindLeastValuableAttacker(
    char occ[8][8],
    Board& board,
    int targetRow, int targetCol,
    bool white,
    int& fromRow, int& fromCol)
{
    fromRow = -1;
    fromCol = -1;
    int bestValue = 1000000;

    auto consider = [&](int rr, int cc, char piece)
    {
        int v = EngineSearch::seeValue(piece);
        if (v < bestValue)
        {
            bestValue = v;
            fromRow = rr;
            fromCol = cc;
        }
    };

    // Pawns: white pawns attack from one row "below" (higher row index,
    // since row 0 = rank 8, row 7 = rank 1, matching Move::parseMove).
    int pawnRow = white ? targetRow + 1 : targetRow - 1;
    for (int dc : { -1, 1 })
    {
        int cc = targetCol + dc;
        if (seeSquareOnBoard(pawnRow, cc))
        {
            char p = occ[pawnRow][cc];
            if (p != '.' && board.isWhitePiece(p) == white && toupper(p) == 'P')
                consider(pawnRow, cc, p);
        }
    }

    // Knights
    static const int knightOffsets[8][2] = {
        {-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}
    };
    for (auto& off : knightOffsets)
    {
        int rr = targetRow + off[0], cc = targetCol + off[1];
        if (seeSquareOnBoard(rr, cc))
        {
            char p = occ[rr][cc];
            if (p != '.' && board.isWhitePiece(p) == white && toupper(p) == 'N')
                consider(rr, cc, p);
        }
    }

    // Bishops/Queens (diagonal rays, stop at first blocker)
    static const int diagDirs[4][2] = { {-1,-1},{-1,1},{1,-1},{1,1} };
    for (auto& dir : diagDirs)
    {
        int r = targetRow + dir[0], c = targetCol + dir[1];
        while (seeSquareOnBoard(r, c))
        {
            char p = occ[r][c];
            if (p != '.')
            {
                if (board.isWhitePiece(p) == white && (toupper(p) == 'B' || toupper(p) == 'Q'))
                    consider(r, c, p);
                break;
            }
            r += dir[0];
            c += dir[1];
        }
    }

    // Rooks/Queens (orthogonal rays, stop at first blocker)
    static const int orthDirs[4][2] = { {-1,0},{1,0},{0,-1},{0,1} };
    for (auto& dir : orthDirs)
    {
        int r = targetRow + dir[0], c = targetCol + dir[1];
        while (seeSquareOnBoard(r, c))
        {
            char p = occ[r][c];
            if (p != '.')
            {
                if (board.isWhitePiece(p) == white && (toupper(p) == 'R' || toupper(p) == 'Q'))
                    consider(r, c, p);
                break;
            }
            r += dir[0];
            c += dir[1];
        }
    }

    // King
    static const int kingOffsets[8][2] = {
        {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
    };
    for (auto& off : kingOffsets)
    {
        int rr = targetRow + off[0], cc = targetCol + off[1];
        if (seeSquareOnBoard(rr, cc))
        {
            char p = occ[rr][cc];
            if (p != '.' && board.isWhitePiece(p) == white && toupper(p) == 'K')
                consider(rr, cc, p);
        }
    }

    return fromRow != -1;
}

int EngineSearch::seeValue(char piece)
{
    switch (toupper(piece))
    {
        case 'P': return 100;
        case 'N': return 320;
        case 'B': return 330;
        case 'R': return 500;
        case 'Q': return 900;
        case 'K': return 20000;
    }
    return 0;
}

// Standard "swap-off" SEE: walks the full capture sequence on the target
// square (both sides always using their cheapest available attacker) and
// returns the net material result for the side making `move`, assuming
// both sides play the exchange optimally.
int EngineSearch::see(Board& board, const Move& move)
{
    char victim = board.getPiece(move.toRow, move.toCol);
    if (victim == '.')
        return 0;   // not a capture (en passant not modeled — treated as 0)

    char occupied[8][8];
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            occupied[r][c] = board.getPiece(r, c);

    int gain[32];
    int d = 0;

    int targetRow = move.toRow;
    int targetCol = move.toCol;

    char attackerPiece = occupied[move.fromRow][move.fromCol];
    bool moverIsWhite = board.isWhitePiece(attackerPiece);

    gain[0] = seeValue(victim);

    occupied[targetRow][targetCol] = attackerPiece;
    occupied[move.fromRow][move.fromCol] = '.';

    bool sideToRecapture = !moverIsWhite;
    int lastAttackerValue = seeValue(attackerPiece);

    while (true)
    {
        int fromRow, fromCol;
        bool found = seeFindLeastValuableAttacker(
            occupied, board, targetRow, targetCol, sideToRecapture, fromRow, fromCol
        );

        if (!found)
            break;

        d++;
        gain[d] = lastAttackerValue - gain[d - 1];

        if (std::max(-gain[d - 1], gain[d]) < 0)
            break;

        char nextAttacker = occupied[fromRow][fromCol];
        lastAttackerValue = seeValue(nextAttacker);

        occupied[targetRow][targetCol] = nextAttacker;
        occupied[fromRow][fromCol] = '.';

        sideToRecapture = !sideToRecapture;

        if (d >= 31)
            break;
    }

    while (d > 0)
    {
        gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
        d--;
    }

    return gain[0];
}

//search the moves
int EngineSearch::negamax(Board& board, int alpha, int beta, int depth, int ply)
{
    nodes++;

    if (stopSearch)
        return 0;

    if ((nodes & 2047) == 0 && checkTimeUp())
    {
        stopSearch = true;
        return 0;
    }

    uint64_t hash = board.getZobristHash();
    int originalAlpha = alpha;
    Move hashMove;
    bool hasHashMove = false;

    //transposition
       //transposition
    TTEntry& ttSlot = transpositionTable[hash & (TT_SIZE - 1)];

    if (ttSlot.hash == hash)
    {
        const TTEntry& entry = ttSlot;

        if (entry.depth >= depth)
        {
            ttHits++;
            hashMove = entry.bestMove;
            hasHashMove = true;

            int ttScore = scoreFromTT(entry.score, ply);

            if (entry.flag == TT_EXACT)
            {
                return ttScore;
            }

            if (entry.flag == TT_ALPHA &&
                ttScore <= alpha)
            {
                return alpha;
            }

            if (entry.flag == TT_BETA &&
                ttScore >= beta)
            {
                return beta;
            }
        }
    }

    // Leaf node
    if (depth == 0)
    {
        return quiescence(board, alpha, beta, 0);
    }

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
        if (board.isKingInCheck(sideToMove))
        {
            return -(MATE_VALUE - ply);   // closer mates score higher in magnitude
        }

        return 0;
    }

    int bestScore = -1000000;
    Move bestMove;
    int moveIndex = 0;

    for (const Move& move : legalMoves)
    {
        // Must check BEFORE making the move — after makeMove, getPiece(to)
        // always shows the mover's own piece, never '.'.
        bool isQuiet = !isCapture(board, move);

        board.makeMove(move);

        bool givesCheckFlag = board.isKingInCheck(board.isWhiteTurn());

        // ---- Futility pruning ----
        if (moveIndex > 0 &&
            depth <= FUTILITY_MAX_DEPTH &&
            !inCheck &&
            isQuiet &&
            !givesCheckFlag &&
            move.promotion == '\0' &&
            !(hasHashMove && move == hashMove) &&
            beta < MATE_THRESHOLD &&
            alpha > -MATE_THRESHOLD)
        {
            int staticEval = Evaluation::boardEvaluation(board);
            int moverEval = board.isWhiteTurn() ? -staticEval : staticEval;
            int margin = FUTILITY_MARGIN_PER_PLY * depth;

            if (moverEval + margin <= alpha)
            {
                board.undoMove();
                moveIndex++;
                continue;
            }
        }

        // ---- Check extension ----
        int extension = givesCheckFlag ? CHECK_EXTENSION : 0;

        int score;

        if (moveIndex == 0)
        {
            // First (best-ordered) move: full window, full depth (+extension).
            score = -negamax(board, -beta, -alpha, depth - 1 + extension, ply + 1);
        }
        else
        {
            // ---- Late Move Reduction ----
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

            int reducedDepth = depth - 1 - reduction + extension;
            if (reducedDepth < 0)
                reducedDepth = 0;

            score = -negamax(board, -alpha - 1, -alpha, reducedDepth, ply + 1);

            if (!stopSearch && score > alpha && reduction > 0)
            {
                score = -negamax(board, -alpha - 1, -alpha, depth - 1 + extension, ply + 1);
            }

            if (!stopSearch && score > alpha && score < beta)
            {
                score = -negamax(board, -beta, -alpha, depth - 1 + extension, ply + 1);
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

        //update alpha
        if (score > alpha)
        {
            alpha = score;
        }

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

  
    //storing tt-entry (depth-preferred replacement)
    if (ttSlot.hash != hash || depth >= ttSlot.depth)
    {
        ttSlot.hash = hash;
        ttSlot.depth = depth;
        ttSlot.score = scoreToTT(bestScore, ply);
        ttSlot.flag = flag;
        ttSlot.bestMove = bestMove;
    }

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

    // Can't stand pat while in check — must search evasions.
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
            // Must consider every legal evasion, regardless of SEE.
            tacticalMoves.push_back(move);
            continue;
        }

        if (capture)
        {
            // ---- SEE-based bad-capture pruning ----
            // Skip captures that lose material outright — searching them
            // deeper almost never changes the verdict and wastes nodes.
            if (see(board, move) < 0)
                continue;

            tacticalMoves.push_back(move);
            continue;
        }

        // Non-capture: only extend with checking moves, and only a
        // limited number of plies, to avoid exploding the tree.
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
            return see(board, a) > see(board, b);
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

    // In check with no legal evasions searched -> checkmate.
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
    long long lastDepthMs = 0;

    for (int depth = 1; depth <= maxdepth; depth++)
    {
        if (depth > 1 &&
            !hasTimeForNextDepth(searchStart, timeLimitMs, lastDepthMs))
        {
            break;
        }

        auto depthStart = std::chrono::steady_clock::now();

        std::vector<Move> legalMoves =
            MoveGenerator::generateLegalMoves(board);

        std::stable_sort(
            legalMoves.begin(),
            legalMoves.end(),
            [&](const Move& a, const Move& b)
            {
                int scoreA = getMoveOrderingScore(board, a, depth);
                int scoreB = getMoveOrderingScore(board, b, depth);

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

                int score = -negamax(board, -searchBeta, -searchAlpha, depth - 1, 1);

                board.undoMove();

                if (stopSearch)
                    break;

                if (score > currentBestScore)
                {
                    currentBestScore = score;
                    currentBestMove = move;
                }

                if (score > searchAlpha)
                    searchAlpha = score;
            }

            if (stopSearch)
                break;

            if (currentBestScore > alpha &&
                currentBestScore < beta)
            {
                break;
            }

            if (currentBestScore <= alpha)
            {
                alpha = -1000000;
            }

            if (currentBestScore >= beta)
            {
                beta = 1000000;
            }
        }

        if (stopSearch)
            break;   // this depth is incomplete — keep the previous depth's move

        bestMove = currentBestMove;
        previousBestMove = bestMove;
        previousScore = currentBestScore;

        auto depthEnd = std::chrono::steady_clock::now();
        lastDepthMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            depthEnd - depthStart
        ).count();
        if (lastDepthMs < 1)
            lastDepthMs = 1;
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

    // Captures: ordered by true exchange value (SEE) instead of MVV-LVA.
    if (isCapture(board, move))
    {
        score += see(board, move) * 10;
    }

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
            if (piece == '.')
                continue;

            bool isWhitePiece = board.isWhitePiece(piece);
            if (isWhitePiece != white)
                continue;

            char upper = toupper(piece);
            if (upper == 'N' || upper == 'B' || upper == 'R' || upper == 'Q')
                return true;
        }
    }

    return false;
}
void EngineSearch::clearTranspositionTable()
{
    for (size_t i = 0; i < TT_SIZE; i++)
        transpositionTable[i] = TTEntry{};
}