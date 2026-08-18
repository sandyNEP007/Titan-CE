#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <vector>
#include "move.h"

class Board;

class MoveGenerator
{
public:
    static std::vector<Move> generateLegalMoves(Board& board);

    private:
    static void generatePawnMoves(Board& board,
        int row,int col,
        std::vector<Move>& moves
    );
    static void generateKnightMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves
);
    static void generateBishopMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves
);
static void generateRookMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves
);
static void generateQueenMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves
);
static void generateKingMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves
);

static void addPromotionMoves(
    int fromRow,
    int fromCol,
    int toRow,
    int toCol,
    std::vector<Move>& moves
);

};

#endif