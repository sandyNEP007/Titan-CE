#ifndef BOARD_H
#define BOARD_H
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>
#include "move.h"
#include "transposition.h"
struct RootMove
{
    Move move;
    int score;
};
struct UndoInfo
{
    Move move;
    Move previousLastMove;

    char movedPiece;

    char capturedPiece;

    bool whiteKingMoved;
    bool blackKingMoved;

    bool whiteLeftRookMoved;
    bool whiteRightRookMoved;

    bool blackLeftRookMoved;
    bool blackRightRookMoved;
};


class Board
{
    private:
    char board[8][8];
    
    Move lastMove;
    std::vector<UndoInfo> history;
    uint64_t zobristTable[12][64];
    uint64_t currentHash;
    std::unordered_map<uint64_t, TTEntry> transpositionTable;
   
public:
    Board();
    void debugSearch(
        const Move& move,
        int depth,
        int score,
        const std::string& action
    );
    void initializeZobristTable();
    uint64_t generateHash();
    int pieceToIndex(char piece);
    std::vector<Move> generateLegalMoves(bool whitePlayer);
    int minimax(int depth, int alpha, int beta, bool maximizingPlayer);
    Move findBestMove(int depth, bool whitePlayer);
    void makeMove(const Move& move);
    bool isValidMove(const Move& move);
    void undoMove(const Move& move);
    void initialize();
    void display();
    void setupPieces();
    bool isWhitePiece(char piece);
    bool isBlackPiece(char piece);
    bool isValidPawnMove(const Move& move);
    bool isValidKnightMove(const Move& move);
    bool isValidBishopMove(const Move& move);
    bool isValidRookMove(const Move& move);
    bool isValidQueenMove(const Move& move);
    bool isValidKingMove(const Move& move);
    bool isKingInCheck(bool whiteKing);
    bool isLegalMove(const Move& move);
    bool hasLegalMove(bool whitePlayer);
    bool whiteKingMoved;
    bool blackKingMoved;

    bool whiteLeftRookMoved;
    bool whiteRightRookMoved;

    bool blackLeftRookMoved;
    bool blackRightRookMoved;

    bool isCheckmate(bool whiteKing);
    bool isStalemate(bool whitePlayer);
    void pawnPromotion();
    void castle();
    void enpassant();
    int BoardEvaluation();
    bool isSquareAttacked(int row, int col, bool byWhite);
    void verifyBoard();
};

#endif