#ifndef BOARD_H
#define BOARD_H
#include<vector>
#include <cstdint>
#include "move.h"
struct UndoInfo
{
    Move move;
    Move previouslastMove;

    char movedPiece;
    char capturedPiece;
    bool whiteKingMoved;
    bool blackKingMoved;

    bool whiteLeftRookMoved;
    bool whiteRightRookMoved;

    bool blackLeftRookMoved;
    bool blackRightRookMoved;
    bool whiteHasCastle;
    bool blackHasCastle;

    bool wasCastling;
    bool previousWhiteTurn;
    uint64_t previousZobristHash;

    bool wasEnPassant;
    char enPassantCapturedPiece;
    int enPassantCapturedRow;
    int enPassantCapturedCol;
    bool previousBlackHasCastled;
    bool previousWhiteHasCastled;
       
};

class Board{
private:
    char board[8][8];
    Move lastMove;
    std::vector<UndoInfo> history;
    uint64_t zobristHash;
    bool whiteKingMoved;
    bool blackKingMoved;

    bool whiteLeftRookMoved;
    bool whiteRightRookMoved;

    bool blackLeftRookMoved;
    bool blackRightRookMoved;
    bool whiteHasCastle;
    bool blackHasCastle;
    std::vector<std::pair<Move, uint64_t>> nullMoveHistory;

public:
    Board();
    char getPiece(int row, int col) const;
    void BoardInitialize();
    void setPieces();
    void display();
    bool isWhiteTurn() const
{
    return whiteTurn;
}

bool hasWhiteCastled() const
{
    return whiteHasCastle;
}

bool hasBlackCastled() const
{
    return blackHasCastle;
}
    bool whiteTurn;
    bool isWhitePiece(char piece);
    bool isBlackPiece(char piece);
    bool isValidMove(const Move& move);
    bool isValidPawnMove(const Move& move);
    bool isValidBishopMove(const Move& move);
    bool isValidKnightMove(const Move& move);
    bool isValidRookMove(const Move& move);
    bool isValidQueenMove(const Move& move);
    bool isValidKingMove(const Move& move);
    bool isValidCastle(const Move& move);
    bool isKingInCheck(bool white);
    bool isSquareAttacked(int row, int col, bool byWhite);
    bool hasLegalMove(bool white);
    bool isCheckmate(bool white);
    bool isStalemate(bool white);
    void makeMove(const Move& move);
    void undoMove();
    void makeNullMove();
    void undoNullMove();
    void setPiece(int row, int col, char piece);
    bool canWhiteKingSideCastle() const;
    bool canWhiteQueenSideCastle() const;
    bool canBlackKingSideCastle() const;
    bool canBlackQueenSideCastle() const;
    int getEnPassantSquare() const;
    uint64_t getZobristHash() const;
   
};
#endif