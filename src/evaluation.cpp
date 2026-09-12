#include "evaluation.h"
#include "board.h"
#include "pst.h"
#include <iostream>

namespace
{
    constexpr int KNIGHT_OUTPOST_BONUS = 30;
    constexpr int OUTPOST_SUPPORTED_BONUS = 10;
    constexpr int OUTPOST_MINOR_ATTACK_PENALTY = 8;
    constexpr int OUTPOST_MAJOR_ATTACK_PENALTY = 5;
}

int Evaluation ::boardEvaluation(Board& board)
{
    int score = 0;

    int whiteBishops = 0;
    int blackBishops = 0;

    // Calculate game phase
    int phase = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            switch (piece)
            {
                case 'N':
                case 'n':
                    phase += 1;
                    break;

                case 'B':
                case 'b':
                    phase += 1;
                    break;

                case 'R':
                case 'r':
                    phase += 2;
                    break;

                case 'Q':
                case 'q':
                    phase += 4;
                    break;
            }
        }
    }

    // Maximum phase = 24
    if (phase > 24)
        phase = 24;

    int endgamePhase = 24 - phase;
    int whiteKingRow = -1;
    int whiteKingCol = -1;
    int blackKingRow = -1;
    int blackKingCol = -1;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == 'K')
            {
                whiteKingRow = row;
                whiteKingCol = col;
            }
            else if (piece == 'k')
            {
                blackKingRow = row;
                blackKingCol = col;
            }
        }
    }


    // Evaluate every square
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            switch (piece)
            {

                case 'P':
                    score += 100;
                    score += PAWN_PST[row][col];
                    break;

                case 'N':
                    score += 320;
                    score += KNIGHT_PST[row][col];
                    break;

                case 'B':
                    whiteBishops++;
                    score += 330;
                    score += BISHOP_PST[row][col];
                    break;

                case 'R':
                    score += 500;
                    score += ROOK_PST[row][col];
                    break;

                case 'Q':
                    score += 900;
                    score += QUEEN_PST[row][col];
                    break;

                case 'K':
                {
                    int mg = KING_MIDDLEGAME_PST[row][col];
                    int eg = KING_ENDGAME_PST[row][col];

                    score += (mg * phase + eg * endgamePhase) / 24;
                    break;
                }

                case 'p':
                    score -= 100;
                    score -= PAWN_PST[7 - row][col];
                    break;

                case 'n':
                    score -= 320;
                    score -= KNIGHT_PST[7 - row][col];
                    break;

                case 'b':
                    blackBishops++;
                    score -= 330;
                    score -= BISHOP_PST[7 - row][col];
                    break;

                case 'r':
                    score -= 500;
                    score -= ROOK_PST[7 - row][col];
                    break;

                case 'q':
                    score -= 900;
                    score -= QUEEN_PST[7 - row][col];
                    break;

                case 'k':
                {
                    int pstRow = 7 - row;

                    int mg = KING_MIDDLEGAME_PST[pstRow][col];
                    int eg = KING_ENDGAME_PST[pstRow][col];

                    int kingScore =
                        (mg * phase + eg * endgamePhase) / 24;

                    score -= kingScore;

                    break;
                }
            }
        }
    }

    if (whiteBishops >= 2)
        score += 30;

    if (blackBishops >= 2)
        score -= 30;

        score += evaluateKnightOutposts(board);
        score += evaluatePawnStructure(board);
        score += evaluateKingMobility(board, phase);
        if (whiteKingRow != -1 && blackKingRow != -1)
        {
            score += evaluatePinnedPieces(board,
                whiteKingRow, whiteKingCol, blackKingRow, blackKingCol);
            score += evaluateEndgameKingActivity(board, endgamePhase,
                whiteKingRow, whiteKingCol, blackKingRow, blackKingCol);
            score += evaluateKingPassedPawnProximity(board, endgamePhase,
                whiteKingRow, whiteKingCol, blackKingRow, blackKingCol);
            score += evaluatePassedPawnSupport(board, endgamePhase,
                whiteKingRow, whiteKingCol, blackKingRow, blackKingCol);
            score += evaluatePromotionThreat(board, endgamePhase,
                whiteKingRow, whiteKingCol, blackKingRow, blackKingCol);
            score += evaluateOpposition(board, endgamePhase,
                whiteKingRow, whiteKingCol, blackKingRow, blackKingCol);
        }
        score += evaluateRookFiles(board, phase);
        score += evaluateRookSeventhRank(board, phase);
        score += evaluateConnectedRooks(board);
        score += evaluateRookBehindPassedPawn(board, endgamePhase);
        score += evaluateGoodBadBishop(board, phase);
        score += evaluateBishopMobility(board, phase);
        score += evaluateBishopLongDiagonal(board, phase);
        score += evaluateKnightMobility(board, phase);
        score += evaluateKnightCentralization(board, endgamePhase);
        score += evaluatePawnStructure(board, endgamePhase);
        score += evaluateProtectedPassedPawns(board);
        score += evaluateConnectedPassedPawns(board);
        score += evaluatePawnAdvancement(board);
        score += evaluateCastling(board, phase);

    return score;
}

int Evaluation::evaluateKnightOutposts(const Board& board)
{
    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            // White knight
            if (piece == 'N')
            {
                if (!isKnightOutpost(board, row, col, true))
                    continue;

                int bonus = KNIGHT_OUTPOST_BONUS;

                // Friendly pawn support
                if (isPawnAttackingSquare(
                        board, row, col, true))
                {
                    bonus += OUTPOST_SUPPORTED_BONUS;
                }

                // Enemy minor-piece attack
                if (isSquareAttackedByMinorPiece(
                        board, row, col, false))
                {
                    bonus -= OUTPOST_MINOR_ATTACK_PENALTY;
                }

                // Enemy rook/queen attack
                if (isSquareAttackedByMajorPiece(
                        board, row, col, false))
                {
                    bonus -= OUTPOST_MAJOR_ATTACK_PENALTY;
                }

                score += bonus;
            }

            // Black knight
            else if (piece == 'n')
            {
                if (!isKnightOutpost(board, row, col, false))
                    continue;

                int bonus = KNIGHT_OUTPOST_BONUS;

                // Friendly pawn support
                if (isPawnAttackingSquare(
                        board, row, col, false))
                {
                    bonus += OUTPOST_SUPPORTED_BONUS;
                }

                // Enemy minor-piece attack
                if (isSquareAttackedByMinorPiece(
                        board, row, col, true))
                {
                    bonus -= OUTPOST_MINOR_ATTACK_PENALTY;
                }

                // Enemy rook/queen attack
                if (isSquareAttackedByMajorPiece(
                        board, row, col, true))
                {
                    bonus -= OUTPOST_MAJOR_ATTACK_PENALTY;
                }

                score -= bonus;
            }
        }
    }

    return score;
}

bool Evaluation::isKnightOutpost(const Board& board, int row, int col, bool whiteKnight)
{
    char piece = board.getPiece(row, col);

    if (whiteKnight && piece != 'N')
        return false;

    if (!whiteKnight && piece != 'n')
        return false;

    // White outpost must be on ranks 4, 5, or 6
    // Black outpost must be on ranks 3, 4, or 5.
    //
    // Internal board:
    // row 0 = rank 8
    // row 7 = rank 1

    if (whiteKnight)
    {
        if (row > 4)
            return false;
    }
    else
    {
        if (row < 3)
            return false;
    }

    // Most important condition:
    // enemy pawn must NOT be able to attack the square.
    if (isPawnAttackingSquare(
            board, row, col, !whiteKnight))
    {
        return false;
    }

    return true;
}

bool Evaluation::isPawnAttackingSquare(const Board& board, int row, int col, bool byWhite)
{
    if (byWhite)
    {
        // White pawn attacks from one row below
        int pawnRow = row + 1;

        if (pawnRow >= 0 && pawnRow < 8)
        {
            if (col - 1 >= 0 &&
                board.getPiece(pawnRow, col - 1) == 'P')
            {
                return true;
            }

            if (col + 1 < 8 &&
                board.getPiece(pawnRow, col + 1) == 'P')
            {
                return true;
            }
        }
    }
    else
    {
        // Black pawn attacks from one row above
        int pawnRow = row - 1;

        if (pawnRow >= 0 && pawnRow < 8)
        {
            if (col - 1 >= 0 &&
                board.getPiece(pawnRow, col - 1) == 'p')
            {
                return true;
            }

            if (col + 1 < 8 &&
                board.getPiece(pawnRow, col + 1) == 'p')
            {
                return true;
            }
        }
    }

    return false;
}

bool Evaluation::isSquareAttackedByMinorPiece(const Board& board, int row, int col, bool byWhite)
{
    char knight = byWhite ? 'N' : 'n';
    char bishop = byWhite ? 'B' : 'b';

    // Knight attacks
    const int knightMoves[8][2] =
    {
        {-2, -1},
        {-2,  1},
        {-1, -2},
        {-1,  2},
        { 1, -2},
        { 1,  2},
        { 2, -1},
        { 2,  1}
    };

    for (const auto& move : knightMoves)
    {
        int r = row + move[0];
        int c = col + move[1];

        if (r >= 0 && r < 8 &&
            c >= 0 && c < 8)
        {
            if (board.getPiece(r, c) == knight)
                return true;
        }
    }

    // Bishop attacks
    const int bishopDirections[4][2] =
    {
        {-1, -1},
        {-1,  1},
        { 1, -1},
        { 1,  1}
    };

    for (const auto& direction : bishopDirections)
    {
        int r = row + direction[0];
        int c = col + direction[1];

        while (r >= 0 && r < 8 &&
               c >= 0 && c < 8)
        {
            char piece = board.getPiece(r, c);

            if (piece != '.')
            {
                if (piece == bishop)
                    return true;

                // Any piece blocks the bishop.
                break;
            }

            r += direction[0];
            c += direction[1];
        }
    }

    return false;
}

bool Evaluation::isSquareAttackedByMajorPiece(const Board& board, int row, int col, bool byWhite)
{
    char rook = byWhite ? 'R' : 'r';
    char queen = byWhite ? 'Q' : 'q';

    const int directions[4][2] =
    {
        {-1, 0},
        { 1, 0},
        { 0,-1},
        { 0, 1}
    };

    for (const auto& direction : directions)
    {
        int r = row + direction[0];
        int c = col + direction[1];

        while (r >= 0 && r < 8 &&
               c >= 0 && c < 8)
        {
            char piece = board.getPiece(r, c);

            if (piece != '.')
            {
                if (piece == rook ||
                    piece == queen)
                {
                    return true;
                }

                // Something blocks the rook/queen.
                break;
            }

            r += direction[0];
            c += direction[1];
        }
    }

    return false;
}

//pawn evaluation 
int Evaluation::evaluateDoubledPawns(const Board& board)
{
    constexpr int DOUBLED_PAWN_PENALTY = 15;

    int score = 0;

    for (int col = 0; col < 8; col++)
    {
        int whitePawns = 0;
        int blackPawns = 0;

        for (int row = 0; row < 8; row++)
        {
            char piece = board.getPiece(row, col);

            if (piece == 'P')
                whitePawns++;

            else if (piece == 'p')
                blackPawns++;
        }

        // Every pawn after the first is doubled.
        if (whitePawns > 1)
        {
            score -= (whitePawns - 1)
                     * DOUBLED_PAWN_PENALTY;
        }

        if (blackPawns > 1)
        {
            score += (blackPawns - 1)
                     * DOUBLED_PAWN_PENALTY;
        }
    }

    return score;
}

int Evaluation::evaluatePawnStructure(const Board& board)
{
    int score = 0;
    score += evaluatePawnChains(board);
    score += evaluatePassedPawns(board);
    return score;
}

int Evaluation::evaluateIsolatedPawns(const Board& board)
{
    constexpr int ISOLATED_PAWN_PENALTY = 12;

    int score = 0;

    for (int col = 0; col < 8; col++)
    {
        bool whitePawnOnFile = false;
        bool blackPawnOnFile = false;

        for (int row = 0; row < 8; row++)
        {
            char piece = board.getPiece(row, col);

            if (piece == 'P')
                whitePawnOnFile = true;

            else if (piece == 'p')
                blackPawnOnFile = true;
        }

        // Check whether the adjacent files contain
        // friendly pawns.
        bool whiteAdjacentPawn = false;
        bool blackAdjacentPawn = false;

        if (col > 0)
        {
            for (int row = 0; row < 8; row++)
            {
                if (board.getPiece(row, col - 1) == 'P')
                    whiteAdjacentPawn = true;

                if (board.getPiece(row, col - 1) == 'p')
                    blackAdjacentPawn = true;
            }
        }

        if (col < 7)
        {
            for (int row = 0; row < 8; row++)
            {
                if (board.getPiece(row, col + 1) == 'P')
                    whiteAdjacentPawn = true;

                if (board.getPiece(row, col + 1) == 'p')
                    blackAdjacentPawn = true;
            }
        }

        if (whitePawnOnFile && !whiteAdjacentPawn)
            score -= ISOLATED_PAWN_PENALTY;

        if (blackPawnOnFile && !blackAdjacentPawn)
            score += ISOLATED_PAWN_PENALTY;
    }

    return score;
}

bool Evaluation::isPawnSupported(const Board& board, int row, int col, bool whitePawn)
{
    if (whitePawn)
    {
        // White pawns move toward decreasing rows.
        // A white pawn behind us is one row lower.
        int supportRow = row + 1;

        if (supportRow >= 8)
            return false;

        if (col > 0 &&
            board.getPiece(supportRow, col - 1) == 'P')
        {
            return true;
        }

        if (col < 7 &&
            board.getPiece(supportRow, col + 1) == 'P')
        {
            return true;
        }
    }
    else
    {
        // Black pawns move toward increasing rows.
        int supportRow = row - 1;

        if (supportRow < 0)
            return false;

        if (col > 0 &&
            board.getPiece(supportRow, col - 1) == 'p')
        {
            return true;
        }

        if (col < 7 &&
            board.getPiece(supportRow, col + 1) == 'p')
        {
            return true;
        }
    }

    return false;
}

int Evaluation::evaluatePawnChains(const Board& board)
{
    constexpr int PAWN_CHAIN_BONUS = 35;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == 'P')
            {
                if (isPawnSupported(board, row, col, true))
                    score += PAWN_CHAIN_BONUS;
            }
            else if (piece == 'p')
            {
                if (isPawnSupported(board, row, col, false))
                    score -= PAWN_CHAIN_BONUS;
            }
        }
    }

    return score;
}

bool Evaluation::isPassedPawn(const Board& board, int row, int col, bool whitePawn)
{
    char enemyPawn = whitePawn ? 'p' : 'P';

    int startRow;
    int endRow;
    int direction;

    if (whitePawn)
    {
        // White moves toward row 0.
        startRow = row - 1;
        endRow = 0;
        direction = -1;
    }
    else
    {
        // Black moves toward row 7.
        startRow = row + 1;
        endRow = 7;
        direction = 1;
    }

    for (int r = startRow;
         whitePawn ? (r >= endRow) : (r <= endRow);
         r += direction)
    {
        // Same file
        if (board.getPiece(r, col) == enemyPawn)
            return false;

        // Left adjacent file
        if (col > 0 &&
            board.getPiece(r, col - 1) == enemyPawn)
            return false;

        // Right adjacent file
        if (col < 7 &&
            board.getPiece(r, col + 1) == enemyPawn)
            return false;
    }

    return true;
}

int Evaluation::evaluatePassedPawns(const Board& board)
{
    int score = 0;

    constexpr int PASSED_PAWN_BONUS[8] =
    {
        0, 10, 15, 20, 30, 45, 70, 0
    };

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == 'P')
            {
                if (isPassedPawn(
                        board, row, col, true))
                {
                    int rank = 8 - row;

                    score += PASSED_PAWN_BONUS[rank - 1];
                }
            }
            else if (piece == 'p')
            {
                if (isPassedPawn(
                        board, row, col, false))
                {
                    int rank = row + 1;

                    score -= PASSED_PAWN_BONUS[rank - 1];
                }
            }
        }
    }

    return score;
}

int Evaluation::evaluateKingMobility(Board& board, int phase)
{
    constexpr int KING_MOBILITY_BONUS = 4;

    int score = 0;
    int mobilityWeight = phase;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece != 'K' && piece != 'k')
                continue;

            bool whiteKing = (piece == 'K');

            int mobility = 0;

            const int directions[8][2] =
            {
                {-1, -1}, {-1, 0}, {-1, 1},
                { 0, -1},           { 0, 1},
                { 1, -1}, { 1, 0}, { 1, 1}
            };

            for (const auto& direction : directions)
            {
                int newRow = row + direction[0];
                int newCol = col + direction[1];

                if (newRow < 0 || newRow >= 8 ||
                    newCol < 0 || newCol >= 8)
                    continue;

                char destination = board.getPiece(newRow, newCol);

                // Cannot move onto a friendly piece.
                if (whiteKing && board.isWhitePiece(destination))
                    continue;

                if (!whiteKing && board.isBlackPiece(destination))
                    continue;

                // King cannot move onto an attacked square.
                Move kingMove(row, col, newRow, newCol);

               bool attacked = board.isSquareAttacked(newRow, newCol, !whiteKing);

                if (!attacked)
                    mobility++;
            }

            int mobilityScore =
                mobility * KING_MOBILITY_BONUS;

            mobilityScore =
                (mobilityScore * mobilityWeight) / 24;

            if (whiteKing)
                score += mobilityScore;
            else
                score -= mobilityScore;
        }
    }

    return score;
}

int Evaluation::evaluateEndgameKingActivity(Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol)
{
    constexpr int KING_ACTIVITY_BONUS = 10;

    // Manhattan distance between kings.
    int distance =
        abs(whiteKingRow - blackKingRow) +
        abs(whiteKingCol - blackKingCol);

    // Maximum useful Manhattan distance is 14.
    int activity =
        (14 - distance) * KING_ACTIVITY_BONUS;

    // Blend according to endgame phase.
    activity =
        (activity * endgamePhase) / 24;

    return activity;
}

int Evaluation::evaluateKingPassedPawnProximity(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol)
{
    constexpr int KING_PAWN_DISTANCE_BONUS = 8;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            // WHITE PASSED PAWN
            if (piece == 'P' &&
                isPassedPawn(board, row, col, true))
            {
                int distance =
                    abs(whiteKingRow - row) +
                    abs(whiteKingCol - col);

                int bonus =
                    14 - distance;

                if (bonus < 0)
                    bonus = 0;

                bonus =
                    (bonus * KING_PAWN_DISTANCE_BONUS)
                    / 14;

                bonus =
                    (bonus * endgamePhase) / 24;

                score += bonus;

                // Black king approaching White's passed pawn
                // has defensive value for Black.
                int enemyDistance =
                    abs(blackKingRow - row) +
                    abs(blackKingCol - col);

                int enemyBonus =
                    14 - enemyDistance;

                if (enemyBonus < 0)
                    enemyBonus = 0;

                enemyBonus =
                    (enemyBonus * KING_PAWN_DISTANCE_BONUS)
                    / 14;

                enemyBonus =
                    (enemyBonus * endgamePhase) / 24;

                score -= enemyBonus;
            }

            // BLACK PASSED PAWN
            else if (piece == 'p' &&
                     isPassedPawn(board, row, col, false))
            {
                int distance =
                    abs(blackKingRow - row) +
                    abs(blackKingCol - col);

                int bonus =
                    14 - distance;

                if (bonus < 0)
                    bonus = 0;

                bonus =
                    (bonus * KING_PAWN_DISTANCE_BONUS)
                    / 14;

                bonus =
                    (bonus * endgamePhase) / 24;

                score -= bonus;

                // White king approaching Black's passed pawn
                // has defensive value for White.
                int enemyDistance =
                    abs(whiteKingRow - row) +
                    abs(whiteKingCol - col);

                int enemyBonus =
                    14 - enemyDistance;

                if (enemyBonus < 0)
                    enemyBonus = 0;

                enemyBonus =
                    (enemyBonus * KING_PAWN_DISTANCE_BONUS)
                    / 14;

                enemyBonus =
                    (enemyBonus * endgamePhase) / 24;

                score += enemyBonus;
            }
        }
    }

    return score;
}

bool Evaluation::isPawnBlockaded(
    const Board& board,
    int row,
    int col,
    bool whitePawn)
{
    int nextRow = whitePawn ? row - 1 : row + 1;

    if (nextRow < 0 || nextRow >= 8)
        return false;

    char piece = board.getPiece(nextRow, col);

    // A piece directly in front of the pawn blocks it.
    if (piece != '.')
        return true;

    return false;
}

int Evaluation::evaluatePassedPawnSupport(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol)
{
  
    constexpr int BLOCKADE_PENALTY = 15;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char pawn = board.getPiece(row, col);

                        // -------------------------------------------------
            // WHITE PASSED PAWN
            // -------------------------------------------------
            // Note: own-king-proximity bonus lives solely in
            // evaluateKingPassedPawnProximity now — only the
            // blockade penalty (a distinct concept) stays here.
            if (pawn == 'P' &&
                isPassedPawn(board, row, col, true))
            {
                // Pawn is directly blocked.
                if (isPawnBlockaded(
                        board, row, col, true))
                {
                    int penalty =
                        (BLOCKADE_PENALTY *
                         endgamePhase) / 24;

                    score -= penalty;
                }
            }

            // -------------------------------------------------
            // BLACK PASSED PAWN
            // -------------------------------------------------
            else if (pawn == 'p' &&
                     isPassedPawn(board, row, col, false))
            {
                if (isPawnBlockaded(
                        board, row, col, false))
                {
                    int penalty =
                        (BLOCKADE_PENALTY *
                         endgamePhase) / 24;

                    score += penalty;
                }
            }
           
        }
    }

    return score;
}

int Evaluation::evaluatePromotionThreat(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol)
{
    constexpr int PROMOTION_BONUS = 100;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            // ------------------------------------------------
            // WHITE PASSED PAWN
            // ------------------------------------------------
            if (piece == 'P' &&
                isPassedPawn(board, row, col, true))
            {
                int rank = 8 - row;

                // Only advanced pawns get this bonus.
                if (rank >= 5)
                {
                    int bonus = (rank - 4) * PROMOTION_BONUS;

                    // Is the black king close enough
                    // to interfere?
                    int enemyKingDistance =
                        std::max(
                            abs(blackKingRow - row),
                            abs(blackKingCol - col));

                    if (enemyKingDistance <= 2)
                    {
                        bonus /= 2;
                    }

                    bonus = (bonus * endgamePhase) / 24;

                    score += bonus;
                }
            }

            // ------------------------------------------------
            // BLACK PASSED PAWN
            // ------------------------------------------------
            else if (piece == 'p' &&
                     isPassedPawn(board, row, col, false))
            {
                int rank = row + 1;

                if (rank >= 5)
                {
                    int bonus =
                        (rank - 4) * PROMOTION_BONUS;

                    int enemyKingDistance =
                        std::max(
                            abs(whiteKingRow - row),
                            abs(whiteKingCol - col));

                    if (enemyKingDistance <= 2)
                    {
                        bonus /= 2;
                    }

                    bonus = (bonus * endgamePhase) / 24;

                    score -= bonus;
                }
            }
        }
    }

    return score;
}

int Evaluation::evaluateOpposition(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol)
{
    constexpr int OPPOSITION_BONUS = 20;

    int score = 0;

    // -------------------------------------------------
    // VERTICAL OPPOSITION
    // -------------------------------------------------

    if (whiteKingCol == blackKingCol &&
        abs(whiteKingRow - blackKingRow) == 2)
    {
       
        if (board.isWhiteTurn())
            score -= OPPOSITION_BONUS;
        else
            score += OPPOSITION_BONUS;
    }

    // -------------------------------------------------
    // HORIZONTAL OPPOSITION
    // -------------------------------------------------

    if (whiteKingRow == blackKingRow &&
        abs(whiteKingCol - blackKingCol) == 2)
    {
        if (board.isWhiteTurn())
            score -= OPPOSITION_BONUS;
        else
            score += OPPOSITION_BONUS;
    }

    // Only relevant in the endgame.
    score =
        (score * endgamePhase) / 24;

    return score;
}

int Evaluation::evaluateRookFiles(const Board& board, int phase)
{
    constexpr int OPEN_FILE_BONUS = 18;
    constexpr int SEMI_OPEN_FILE_BONUS = 10;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char rook = board.getPiece(row, col);

            if (rook != 'R' && rook != 'r')
                continue;

            bool whiteRook = (rook == 'R');

            bool whitePawnOnFile = false;
            bool blackPawnOnFile = false;

            for (int fileRow = 0; fileRow < 8; fileRow++)
            {
                char piece =
                    board.getPiece(fileRow, col);

                if (piece == 'P')
                    whitePawnOnFile = true;

                if (piece == 'p')
                    blackPawnOnFile = true;
            }

            int bonus = 0;

            // No pawns at all = open file.
            if (!whitePawnOnFile &&
                !blackPawnOnFile)
            {
                bonus = OPEN_FILE_BONUS;
            }

            // No friendly pawn = semi-open file.
            else if (whiteRook && !whitePawnOnFile)
            {
                bonus = SEMI_OPEN_FILE_BONUS;
            }
            else if (!whiteRook && !blackPawnOnFile)
            {
                bonus = SEMI_OPEN_FILE_BONUS;
            }

           
            int weight = 24 - phase;

            int scaledBonus =
                bonus + (bonus * weight) / 48;

            if (whiteRook)
                score += scaledBonus;
            else
                score -= scaledBonus;
        }
    }

    return score;
}

int Evaluation::evaluateRookSeventhRank(const Board& board, int phase)
{
    constexpr int SEVENTH_RANK_BONUS = 20;

    int score = 0;

    bool blackPawnOnSeventh = false;
    bool whitePawnOnSecond = false;

    // White's 7th rank = row 1
    // Black's 2nd rank = row 6

    for (int col = 0; col < 8; col++)
    {
        if (board.getPiece(1, col) == 'p')
            blackPawnOnSeventh = true;

        if (board.getPiece(6, col) == 'P')
            whitePawnOnSecond = true;
    }

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            // White rook on 7th rank.
            if (piece == 'R' &&
                row == 1 &&
                blackPawnOnSeventh)
            {
                int bonus = (SEVENTH_RANK_BONUS * (24 - phase)) / 24;

                score += bonus;
            }

            // Black rook on 2nd rank.
            if (piece == 'r' &&
                row == 6 &&
                whitePawnOnSecond)
            {
                int bonus = (SEVENTH_RANK_BONUS * (24 - phase))/ 24;
                score -= bonus;
            }
        }
    }

    return score;
}

int Evaluation::evaluateConnectedRooks(const Board& board)
{
    constexpr int CONNECTED_ROOK_BONUS = 12;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char rook = board.getPiece(row, col);

            if (rook != 'R' && rook != 'r')
                continue;

            bool whiteRook = (rook == 'R');

            // -----------------------------------------
            // Same rank
            // -----------------------------------------

            for (int otherCol = col + 1;
                 otherCol < 8;
                 otherCol++)
            {
                char other =
                    board.getPiece(row, otherCol);

                if (other != '.')
                {
                    if ((whiteRook && other == 'R') ||
                        (!whiteRook && other == 'r'))
                    {
                        score += whiteRook
                            ? CONNECTED_ROOK_BONUS
                            : -CONNECTED_ROOK_BONUS;
                    }

                    break;
                }
            }

            // -----------------------------------------
            // Same file
            // -----------------------------------------

            for (int otherRow = row + 1;
                 otherRow < 8;
                 otherRow++)
            {
                char other =
                    board.getPiece(otherRow, col);

                if (other != '.')
                {
                    if ((whiteRook && other == 'R') ||
                        (!whiteRook && other == 'r'))
                    {
                        score += whiteRook
                            ? CONNECTED_ROOK_BONUS
                            : -CONNECTED_ROOK_BONUS;
                    }

                    break;
                }
            }
        }
    }

    return score;
}

int Evaluation::evaluateRookBehindPassedPawn(const Board& board, int endgamePhase)
{
    constexpr int ROOK_BEHIND_PASSED_PAWN_BONUS = 20;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char pawn = board.getPiece(row, col);

            // -------------------------------------------------
            // WHITE PASSED PAWN
            // -------------------------------------------------

            if (pawn == 'P' &&
                isPassedPawn(board, row, col, true))
            {


                for (int rookRow = row + 1;
                     rookRow < 8;
                     rookRow++)
                {
                    char piece =
                        board.getPiece(rookRow, col);

                    if (piece == 'R')
                    {
                        int pawnRank = 8 - row;

                        int bonus =
                            ROOK_BEHIND_PASSED_PAWN_BONUS;

                        // More advanced pawn = more useful.
                        if (pawnRank >= 6)
                            bonus += 8;
                        else if (pawnRank >= 5)
                            bonus += 4;

                        bonus =
                            (bonus * endgamePhase) / 24;

                        score += bonus;

                        break;
                    }

                    // Another piece blocks the rook.
                    if (piece != '.')
                        break;
                }
            }

            // -------------------------------------------------
            // BLACK PASSED PAWN
            // -------------------------------------------------

            else if (pawn == 'p' &&
                     isPassedPawn(board, row, col, false))
            {

                for (int rookRow = row - 1;
                     rookRow >= 0;
                     rookRow--)
                {
                    char piece =
                        board.getPiece(rookRow, col);

                    if (piece == 'r')
                    {
                        int pawnRank = row + 1;

                        int bonus =
                            ROOK_BEHIND_PASSED_PAWN_BONUS;

                        if (pawnRank >= 6)
                            bonus += 8;
                        else if (pawnRank >= 5)
                            bonus += 4;

                        bonus =
                            (bonus * endgamePhase) / 24;

                        score -= bonus;

                        break;
                    }

                    // Another piece blocks the rook.
                    if (piece != '.')
                        break;
                }
            }
        }
    }

    return score;
}

int Evaluation::evaluateRookMobility(Board& board, int phase)
{
    constexpr int MOBILITY_BONUS = 2;
    constexpr int PAWN_ATTACK_BONUS = 3;
    constexpr int PASSED_PAWN_ATTACK_BONUS = 5;

    int score = 0;

    const int directions[4][2] =
    {
        {-1, 0},   // up
        { 1, 0},   // down
        { 0,-1},   // left
        { 0, 1}    // right
    };

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char rook = board.getPiece(row, col);

            if (rook != 'R' && rook != 'r')
                continue;

            bool whiteRook = (rook == 'R');

            int mobility = 0;
            int usefulBonus = 0;

            for (int direction = 0;
                 direction < 4;
                 direction++)
            {
                int newRow =
                    row + directions[direction][0];

                int newCol =
                    col + directions[direction][1];

                while (newRow >= 0 && newRow < 8 &&
                       newCol >= 0 && newCol < 8)
                {
                    char destination =
                        board.getPiece(newRow, newCol);

                    // Friendly piece blocks the rook.
                    if (whiteRook &&
                        board.isWhitePiece(destination))
                    {
                        break;
                    }

                    if (!whiteRook &&
                        board.isBlackPiece(destination))
                    {
                        break;
                    }

                    // Empty square.
                    if (destination == '.')
                    {
                        mobility++;
                    }

                    // Enemy piece.
                    else
                    {
                        // Capturable enemy pawn.
                        if (whiteRook &&
                            destination == 'p')
                        {
                            usefulBonus +=
                                PAWN_ATTACK_BONUS;

                            if (isPassedPawn(
                                    board,
                                    newRow,
                                    newCol,
                                    false))
                            {
                                usefulBonus +=
                                    PASSED_PAWN_ATTACK_BONUS;
                            }
                        }

                        else if (!whiteRook &&
                                 destination == 'P')
                        {
                            usefulBonus +=
                                PAWN_ATTACK_BONUS;

                            if (isPassedPawn(
                                    board,
                                    newRow,
                                    newCol,
                                    true))
                            {
                                usefulBonus +=
                                    PASSED_PAWN_ATTACK_BONUS;
                            }
                        }

                        // Rook cannot move beyond an enemy piece.
                        break;
                    }

                    newRow += directions[direction][0];
                    newCol += directions[direction][1];
                }
            }

            int bonus = mobility * MOBILITY_BONUS;

            bonus += usefulBonus;

            int endgameWeight = 24 - phase;

            bonus +=
                (bonus * endgameWeight) / 48;

            if (whiteRook)
                score += bonus;
            else
                score -= bonus;
        }
    }

    return score;
}

int Evaluation::evaluateBishopMobility(Board& board, int phase)
{
    constexpr int MOBILITY_BONUS = 2;
    constexpr int PAWN_ATTACK_BONUS = 3;

    int score = 0;

    const int directions[4][2] =
    {
        {-1, -1},
        {-1,  1},
        { 1, -1},
        { 1,  1}
    };

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char bishop = board.getPiece(row, col);

            if (bishop != 'B' && bishop != 'b')
                continue;

            bool whiteBishop = (bishop == 'B');

            int mobility = 0;
            int usefulBonus = 0;

            for (int direction = 0;
                 direction < 4;
                 direction++)
            {
                int newRow =
                    row + directions[direction][0];

                int newCol =
                    col + directions[direction][1];

                while (newRow >= 0 && newRow < 8 &&
                       newCol >= 0 && newCol < 8)
                {
                    char destination =
                        board.getPiece(newRow, newCol);

                    // Friendly piece blocks bishop.
                    if (whiteBishop &&
                        board.isWhitePiece(destination))
                    {
                        break;
                    }

                    if (!whiteBishop &&
                        board.isBlackPiece(destination))
                    {
                        break;
                    }

                    // Empty square.
                    if (destination == '.')
                    {
                        mobility++;
                    }
                    else
                    {
                        // Enemy pawn is a useful target.
                        if (whiteBishop &&
                            destination == 'p')
                        {
                            usefulBonus +=
                                PAWN_ATTACK_BONUS;
                        }
                        else if (!whiteBishop &&
                                 destination == 'P')
                        {
                            usefulBonus +=
                                PAWN_ATTACK_BONUS;
                        }

                        // Enemy piece blocks further movement.
                        break;
                    }

                    newRow += directions[direction][0];
                    newCol += directions[direction][1];
                }
            }

            int bonus =
                mobility * MOBILITY_BONUS;

            bonus += usefulBonus;

            // Slightly emphasize bishop activity
            // as the board becomes less crowded.
            int endgameWeight = 24 - phase;

            bonus +=
                (bonus * endgameWeight) / 48;

            if (whiteBishop)
                score += bonus;
            else
                score -= bonus;
        }
    }

    return score;
}

int Evaluation::evaluateGoodBadBishop(const Board& board, int phase)
{
    constexpr int BAD_BISHOP_PENALTY = 8;
    constexpr int GOOD_BISHOP_BONUS = 4;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char bishop = board.getPiece(row, col);

            if (bishop != 'B' && bishop != 'b')
                continue;

            bool whiteBishop = (bishop == 'B');

            // Determine bishop square color.
            int bishopColor = (row + col) % 2;

            int sameColorPawns = 0;
            int totalFriendlyPawns = 0;

            for (int pawnRow = 0;
                 pawnRow < 8;
                 pawnRow++)
            {
                for (int pawnCol = 0;
                     pawnCol < 8;
                     pawnCol++)
                {
                    char pawn =
                        board.getPiece(pawnRow, pawnCol);

                    if (whiteBishop && pawn == 'P')
                    {
                        totalFriendlyPawns++;

                        if ((pawnRow + pawnCol) % 2 ==
                            bishopColor)
                        {
                            sameColorPawns++;
                        }
                    }

                    else if (!whiteBishop && pawn == 'p')
                    {
                        totalFriendlyPawns++;

                        if ((pawnRow + pawnCol) % 2 ==
                            bishopColor)
                        {
                            sameColorPawns++;
                        }
                    }
                }
            }

            if (totalFriendlyPawns == 0)
                continue;


            if (sameColorPawns >= 3)
            {
                int penalty =
                    BAD_BISHOP_PENALTY;

                int endgameWeight = 24 - phase;

                penalty +=
                    (penalty * endgameWeight) / 48;

                if (whiteBishop)
                    score -= penalty;
                else
                    score += penalty;
            }
            else if (sameColorPawns == 0)
            {
                int bonus =
                    GOOD_BISHOP_BONUS;

                int endgameWeight = 24 - phase;

                bonus +=
                    (bonus * endgameWeight) / 48;

                if (whiteBishop)
                    score += bonus;
                else
                    score -= bonus;
            }
        }
    }

    return score;
}

int Evaluation::evaluateBishopLongDiagonal(Board& board, int phase)
{
    constexpr int LONG_DIAGONAL_BONUS = 8;

    int score = 0;

    const int directions[4][2] =
    {
        {-1, -1},
        {-1,  1},
        { 1, -1},
        { 1,  1}
    };

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char bishop = board.getPiece(row, col);

            if (bishop != 'B' && bishop != 'b')
                continue;

            bool whiteBishop = (bishop == 'B');

            int longestDiagonal = 0;

            for (int direction = 0;
                 direction < 4;
                 direction++)
            {
                int length = 0;

                int newRow =
                    row + directions[direction][0];

                int newCol =
                    col + directions[direction][1];

                while (newRow >= 0 && newRow < 8 &&
                       newCol >= 0 && newCol < 8)
                {
                    char destination =
                        board.getPiece(newRow, newCol);

                    // Friendly piece blocks the diagonal.
                    if (whiteBishop &&
                        board.isWhitePiece(destination))
                    {
                        break;
                    }

                    if (!whiteBishop &&
                        board.isBlackPiece(destination))
                    {
                        break;
                    }

                    length++;

                    if (destination != '.')
                        break;

                    newRow += directions[direction][0];
                    newCol += directions[direction][1];
                }

                if (length > longestDiagonal)
                    longestDiagonal = length;
            }

            if (longestDiagonal >= 5)
            {
                int bonus =
                    LONG_DIAGONAL_BONUS;

                // Reward especially long diagonals.
                if (longestDiagonal >= 6)
                    bonus += 2;

                if (longestDiagonal >= 7)
                    bonus += 2;

                // Slightly more important as material
                // disappears.
                int endgameWeight = 24 - phase;

                bonus +=
                    (bonus * endgameWeight) / 48;

                if (whiteBishop)
                    score += bonus;
                else
                    score -= bonus;
            }
        }
    }

    return score;
}

int Evaluation::evaluateKnightMobility(Board& board, int phase)
{
    constexpr int MOBILITY_BONUS = 2;
    constexpr int PAWN_ATTACK_BONUS = 2;

    const int knightMoves[8][2] =
    {
        {-2, -1},
        {-2,  1},
        {-1, -2},
        {-1,  2},
        { 1, -2},
        { 1,  2},
        { 2, -1},
        { 2,  1}
    };

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char knight = board.getPiece(row, col);

            if (knight != 'N' && knight != 'n')
                continue;

            bool whiteKnight = (knight == 'N');

            int mobility = 0;
            int usefulTargets = 0;

            for (int i = 0; i < 8; i++)
            {
                int newRow =
                    row + knightMoves[i][0];

                int newCol =
                    col + knightMoves[i][1];

                if (newRow < 0 || newRow >= 8 ||
                    newCol < 0 || newCol >= 8)
                    continue;

                char destination =
                    board.getPiece(newRow, newCol);

                // Friendly piece occupies square.
                if (whiteKnight &&
                    board.isWhitePiece(destination))
                    continue;

                if (!whiteKnight &&
                    board.isBlackPiece(destination))
                    continue;

                mobility++;

                // Enemy pawn is a useful target.
                if (whiteKnight && destination == 'p')
                    usefulTargets += PAWN_ATTACK_BONUS;

                if (!whiteKnight && destination == 'P')
                    usefulTargets += PAWN_ATTACK_BONUS;
            }

            int bonus =
                mobility * MOBILITY_BONUS;

            bonus += usefulTargets;

            // Knight mobility becomes more important
            // in simplified positions.
            int endgameWeight = 24 - phase;

            bonus +=
                (bonus * endgameWeight) / 48;

            if (whiteKnight)
                score += bonus;
            else
                score -= bonus;
        }
    }

    return score;
}

int Evaluation::evaluateKnightCentralization(const Board& board, int endgamePhase)
{
    constexpr int CENTER_BONUS = 8;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char knight = board.getPiece(row, col);

            if (knight != 'N' && knight != 'n')
                continue;

            bool whiteKnight = (knight == 'N');

            int distanceToCenter =
                std::min(
                    std::min(
                        abs(row - 3) +
                        abs(col - 3),

                        abs(row - 3) +
                        abs(col - 4)
                    ),

                    std::min(
                        abs(row - 4) +
                        abs(col - 3),

                        abs(row - 4) +
                        abs(col - 4)
                    )
                );

            int bonus = 0;

            if (distanceToCenter == 0)
                bonus = CENTER_BONUS;
            else if (distanceToCenter == 1)
                bonus = 5;
            else if (distanceToCenter == 2)
                bonus = 2;

            /*
                Centralization becomes more important
                as the board empties.
            */

            bonus =
                (bonus * endgamePhase) / 24;

            if (whiteKnight)
                score += bonus;
            else
                score -= bonus;
        }
    }

    return score;
}

int Evaluation::evaluatePawnStructure(const Board& board, int endgamePhase)
{
    constexpr int DOUBLED_PAWN_PENALTY = 10;
    constexpr int ISOLATED_PAWN_PENALTY = 12;

    int score = 0;

    int whitePawnsOnFile[8] = {};
    int blackPawnsOnFile[8] = {};

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == 'P')
                whitePawnsOnFile[col]++;

            else if (piece == 'p')
                blackPawnsOnFile[col]++;
        }
    }

    // Doubled pawns
    for (int col = 0; col < 8; col++)
    {
        if (whitePawnsOnFile[col] > 1)
        {
            score -= (whitePawnsOnFile[col] - 1) * DOUBLED_PAWN_PENALTY;
        }

        if (blackPawnsOnFile[col] > 1)
        {
            score += (blackPawnsOnFile[col] - 1) * DOUBLED_PAWN_PENALTY;
        }
    }

    // Isolated pawns
    for (int col = 0; col < 8; col++)
    {
        bool whiteHasNeighbor =
            (col > 0 &&
             whitePawnsOnFile[col - 1] > 0) ||
            (col < 7 &&
             whitePawnsOnFile[col + 1] > 0);

        bool blackHasNeighbor =
            (col > 0 &&
             blackPawnsOnFile[col - 1] > 0) ||
            (col < 7 &&
             blackPawnsOnFile[col + 1] > 0);

        if (whitePawnsOnFile[col] > 0 &&
            !whiteHasNeighbor)
        {
            score -= ISOLATED_PAWN_PENALTY;
        }

        if (blackPawnsOnFile[col] > 0 &&
            !blackHasNeighbor)
        {
            score += ISOLATED_PAWN_PENALTY;
        }
    }

    // Pawn weaknesses become more important
    // as the game approaches the endgame.
    score =
        (score * (12 + endgamePhase)) / 24;

    return score;
}


int Evaluation::evaluateProtectedPassedPawns(const Board& board)
{
    constexpr int PROTECTED_PASSED_PAWN_BONUS = 12;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char pawn = board.getPiece(row, col);

            // -------------------------------------------------
            // WHITE
            // -------------------------------------------------

            if (pawn == 'P' &&
                isPassedPawn(board, row, col, true))
            {

                int supportRow = row + 1;

                if (supportRow < 8)
                {
                    bool protectedPawn = false;

                    if (col > 0 &&
                        board.getPiece(
                            supportRow,
                            col - 1) == 'P')
                    {
                        protectedPawn = true;
                    }

                    if (col < 7 &&
                        board.getPiece(supportRow, col + 1) == 'P')
                    {
                        protectedPawn = true;
                    }

                    if (protectedPawn)
                        score += PROTECTED_PASSED_PAWN_BONUS;
                }
            }

            // -------------------------------------------------
            // BLACK
            // -------------------------------------------------

            else if (pawn == 'p' &&isPassedPawn(board,row, col, false))
            {

                int supportRow = row - 1;

                if (supportRow >= 0)
                {
                    bool protectedPawn = false;

                    if (col > 0 &&
                        board.getPiece(
                            supportRow,
                            col - 1) == 'p')
                    {
                        protectedPawn = true;
                    }

                    if (col < 7 &&
                        board.getPiece(
                            supportRow,
                            col + 1) == 'p')
                    {
                        protectedPawn = true;
                    }

                    if (protectedPawn)
                        score -=
                            PROTECTED_PASSED_PAWN_BONUS;
                }
            }
        }
    }

    return score;
}

int Evaluation::evaluateConnectedPassedPawns(const Board& board)
{
    constexpr int CONNECTED_PASSED_PAWN_BONUS = 18;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 7; col++)
        {
            char pawn1 = board.getPiece(row, col);
            char pawn2 = board.getPiece(row, col + 1);

            // -------------------------------------------------
            // WHITE CONNECTED PASSED PAWNS
            // -------------------------------------------------

            if (pawn1 == 'P' && pawn2 == 'P')
            {
                bool pawn1Passed = isPassedPawn(board, row, col, true);

                bool pawn2Passed =
                    isPassedPawn(board, row, col + 1, true);

                if (pawn1Passed && pawn2Passed)
                {
                    score += CONNECTED_PASSED_PAWN_BONUS;
                }
            }

            // -------------------------------------------------
            // BLACK CONNECTED PASSED PAWNS
            // -------------------------------------------------

            else if (pawn1 == 'p' && pawn2 == 'p')
            {
                bool pawn1Passed =isPassedPawn(board, row, col, false);

                bool pawn2Passed =isPassedPawn(board, row, col + 1, false);
                if (pawn1Passed && pawn2Passed)
                {
                    score -= CONNECTED_PASSED_PAWN_BONUS;
                }
            }
        }
    }

    return score;
}

int Evaluation::evaluatePawnAdvancement(
    const Board& board)
{
    constexpr int ADVANCEMENT_BONUS = 4;

    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char pawn = board.getPiece(row, col);

            if (pawn == 'P' &&
                isPassedPawn(board, row, col, true))
            {

                int rank = 8 - row;

                int bonus = 0;

                if (rank == 7)
                    bonus = 16;
                else if (rank == 6)
                    bonus = 12;
                else if (rank == 5)
                    bonus = 8;
                else if (rank == 4)
                    bonus = 5;
                else if (rank == 3)
                    bonus = 3;
                else
                    bonus = ADVANCEMENT_BONUS;

                score += bonus;
            }

            else if (pawn == 'p' &&
                     isPassedPawn(
                         board,
                         row,
                         col,
                         false))
            {
                int rank = row + 1;

                int bonus = 0;

                if (rank == 7)
                    bonus = 16;
                else if (rank == 6)
                    bonus = 12;
                else if (rank == 5)
                    bonus = 8;
                else if (rank == 4)
                    bonus = 5;
                else if (rank == 3)
                    bonus = 3;
                else
                    bonus = ADVANCEMENT_BONUS;

                score -= bonus;
            }
        }
    }

    return score;
}

int Evaluation::evaluateCastling(Board& board, int phase)
{
       constexpr int CASTLING_BONUS = 40;
       int score = 0;
       int bonus = (CASTLING_BONUS * phase) / 24;

    if (board.hasWhiteCastled())
        score += bonus;

    if (board.hasBlackCastled())
        score -= bonus;

    return score;
}
int Evaluation::isPinned(Board &board, int row, int col, bool byWhite,
    int ownKingRow, int ownKingCol)
{
    constexpr int PINNED_PIECE_PENALTY = 50;
    int score = 0;

    if (row == ownKingRow && col == ownKingCol)
        return score;   // the king itself is never "pinned"

    int dRow = row - ownKingRow;
    int dCol = col - ownKingCol;

    bool sameRow = (dRow == 0);
    bool sameCol = (dCol == 0);
    bool diagonal = (!sameRow && !sameCol && abs(dRow) == abs(dCol));

    if (!sameRow && !sameCol && !diagonal)
        return score;   // not aligned with own king — can't be pinned

    int stepRow = (dRow == 0) ? 0 : (dRow > 0 ? 1 : -1);
    int stepCol = (dCol == 0) ? 0 : (dCol > 0 ? 1 : -1);

    // Walk from the king toward (row,col) — our piece must be the FIRST
    // one on this ray, or something else is already blocking it.
    int r = ownKingRow + stepRow;
    int c = ownKingCol + stepCol;

    while (r != row || c != col)
    {
        if (board.getPiece(r, c) != '.')
            return score;   // blocked before reaching our piece

        r += stepRow;
        c += stepCol;
    }

    // Continue past our piece, looking for an enemy slider of the
    // matching type with nothing else in between.
    r += stepRow;
    c += stepCol;

    char enemySlider = diagonal
        ? (byWhite ? 'b' : 'B')
        : (byWhite ? 'r' : 'R');

    char enemyQueen = byWhite ? 'q' : 'Q';

    while (r >= 0 && r < 8 && c >= 0 && c < 8)
    {
        char piece = board.getPiece(r, c);

        if (piece == '.')
        {
            r += stepRow;
            c += stepCol;
            continue;
        }

        if (piece == enemySlider || piece == enemyQueen)
        {
            score = byWhite ? -PINNED_PIECE_PENALTY : PINNED_PIECE_PENALTY;
        }

        break;
    }

    return score;
}

int Evaluation::evaluatePinnedPieces(Board& board,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol)
{
    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == '.')
                continue;

            bool isWhitePiece = board.isWhitePiece(piece);

            // Kings can't be pinned, and pinning your own king to itself
            // makes no sense — skip both king squares.
            if (piece == 'K' || piece == 'k')
                continue;

            if (isWhitePiece)
                score += isPinned(board, row, col, true, whiteKingRow, whiteKingCol);
            else
                score += isPinned(board, row, col, false, blackKingRow, blackKingCol);
        }
    }

    return score;
}