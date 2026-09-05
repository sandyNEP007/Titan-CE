#ifndef EVALUATION_H
#define EVALUATION_H

#include "board.h"

class Evaluation
{
public:
    static int evaluateKnightOutposts(const Board& board);
    static int boardEvaluation(Board& board);
    static int evaluatePawnStructure(const Board& board);
    static int evaluateDoubledPawns(const Board& board);
    static int evaluateIsolatedPawns(const Board& board);
    static int evaluatePassedPawns(const Board& board);
    static bool isPassedPawn(const Board& board, int row, int col, bool whitePawn);
    static bool isPawnSupported(const Board& board, int row, int col, bool whitePawn);
    static int evaluateKingMobility(Board& board, int phase);
    static int evaluateEndgameKingActivity(Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol);
    static int evaluateKingPassedPawnProximity(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol);
    static int evaluatePassedPawnSupport(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol);
    static int evaluatePawnChains(const Board& board);
    static bool isPawnBlockaded(const Board& board, int row, int col, bool whitePawn);
   static int evaluatePromotionThreat(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol);
   static int evaluateOpposition(const Board& board, int endgamePhase,
    int whiteKingRow, int whiteKingCol, int blackKingRow, int blackKingCol);
    static int evaluateRookFiles(const Board& board, int phase);
    static int evaluateRookSeventhRank(const Board& board, int phase);
    static int evaluateConnectedRooks(const Board& board);
    static int evaluateRookBehindPassedPawn(const Board& board,int endgamePhase);
    static int evaluateRookMobility(Board& board, int phase);
    static int evaluateBishopMobility(Board& board, int phase);
    static int evaluateGoodBadBishop(const Board& board, int phase);
    static int evaluateBishopLongDiagonal(Board& board, int phase);
    static int evaluateKnightMobility(Board& board, int phase);
    static int evaluateKnightCentralization(const Board& board, int endgamePhase);
    static int evaluatePawnStructure(const Board& board, int endgamePhase);
    static int evaluateProtectedPassedPawns(const Board& board);
    static int evaluateConnectedPassedPawns(const Board& board);
    static int evaluatePawnAdvancement(const Board& board);
    static int evaluateCastling(Board& board, int phase);

private:
    static int getPieceValue(char piece);
    static int getGamePhase(const Board& board);
    static bool isKnightOutpost(const Board& board, int row, int col, bool whiteKnight);
    static bool isPawnAttackingSquare(const Board& board, int row, int col, bool byWhite);
    static bool isSquareAttackedByMinorPiece(const Board& board, int row, int col, bool byWhite );
    static bool isSquareAttackedByMajorPiece(const Board& board, int row,int col,bool byWhite);

};

#endif