#include<iostream>
#include "movegen.h"
#include "board.h"

std::vector<Move> MoveGenerator::generateLegalMoves(Board& board)
{
    std::vector<Move> legalMoves;
    std::vector<Move> pseudoLegalMoves;

    bool movingSide = board.isWhiteTurn();

    // Generate pseudo-legal moves
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == '.')
                continue;

            if (movingSide)
            {
                if (!(piece >= 'A' && piece <= 'Z'))
                    continue;
            }
            else
            {
                if (!(piece >= 'a' && piece <= 'z'))
                    continue;
            }

            if (piece == 'P' || piece == 'p')
                generatePawnMoves(board, row, col, pseudoLegalMoves);

            else if (piece == 'N' || piece == 'n')
                generateKnightMoves(board, row, col, pseudoLegalMoves);

            else if (piece == 'B' || piece == 'b')
                generateBishopMoves(board, row, col, pseudoLegalMoves);

            else if (piece == 'R' || piece == 'r')
                generateRookMoves(board, row, col, pseudoLegalMoves);

            else if (piece == 'Q' || piece == 'q')
                generateQueenMoves(board, row, col, pseudoLegalMoves);

            else if (piece == 'K' || piece == 'k')
                generateKingMoves(board, row, col, pseudoLegalMoves);
        }
    }

    // Remove moves that leave our own king in check
    for (const Move& move : pseudoLegalMoves)
    {
        board.makeMove(move);

        bool leavesKingInCheck =
            board.isKingInCheck(movingSide);

        board.undoMove();

        if (!leavesKingInCheck)
        {
            legalMoves.push_back(move);
        }
    }

    return legalMoves;
}

void MoveGenerator::addPromotionMoves(
    int fromRow,
    int fromCol,
    int toRow,
    int toCol,
    std::vector<Move>& moves)
{
    moves.push_back(
        Move(fromRow, fromCol, toRow, toCol, 'Q')
    );

    moves.push_back(
        Move(fromRow, fromCol, toRow, toCol, 'R')
    );

    moves.push_back(
        Move(fromRow, fromCol, toRow, toCol, 'B')
    );

    moves.push_back(
        Move(fromRow, fromCol, toRow, toCol, 'N')
    );
}

//Generating pseudo-legal move of pawn
void MoveGenerator::generatePawnMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves)
{
    char piece = board.getPiece(row, col);
    

    // White pawn
    if (piece == 'P')
    {
        // One square forward
      if (row - 1 >= 0 &&
    board.getPiece(row - 1, col) == '.')
{
    // Promotion
    if (row - 1 == 0)
    {
        moves.push_back(Move(row, col, row - 1, col, 'Q'));
        moves.push_back(Move(row, col, row - 1, col, 'R'));
        moves.push_back(Move(row, col, row - 1, col, 'B'));
        moves.push_back(Move(row, col, row - 1, col, 'N'));
    }
    else
    {
        // Normal pawn move
        moves.push_back(
            Move(row, col, row - 1, col)
        );

        // Two squares from starting rank
        if (row == 6 &&
            board.getPiece(row - 2, col) == '.')
        {
            moves.push_back(
                Move(row, col, row - 2, col)
            );
        }
    }
}
        // Capture diagonally left
       if (row - 1 >= 0 && col - 1 >= 0 &&
    board.isBlackPiece(board.getPiece(row - 1, col - 1)))
{
    // Capture promotion
    if (row - 1 == 0)
    {
        moves.push_back(
            Move(row, col, row - 1, col - 1, 'Q')
        );

        moves.push_back(
            Move(row, col, row - 1, col - 1, 'R')
        );

        moves.push_back(
            Move(row, col, row - 1, col - 1, 'B')
        );

        moves.push_back(
            Move(row, col, row - 1, col - 1, 'N')
        );
    }
    else
    {
        moves.push_back(
            Move(row, col, row - 1, col - 1)
        );
    }
}

        // Capture diagonally right
       if (row - 1 >= 0 && col + 1 <= 7 &&
    board.isBlackPiece(board.getPiece(row - 1, col + 1)))
{
    // Capture promotion
    if (row - 1 == 0)
    {
        moves.push_back(
            Move(row, col, row - 1, col + 1, 'Q')
        );

        moves.push_back(
            Move(row, col, row - 1, col + 1, 'R')
        );

        moves.push_back(
            Move(row, col, row - 1, col + 1, 'B')
        );

        moves.push_back(
            Move(row, col, row - 1, col + 1, 'N')
        );
    }
    else
    {
        moves.push_back(
            Move(row, col, row - 1, col + 1)
        );
    }
 }
 // White en passant
int enPassantSquare = board.getEnPassantSquare();

if (enPassantSquare != -1)
{
    int epRow = enPassantSquare / 8;
    int epCol = enPassantSquare % 8;

    // White pawn must be on the 5th rank
    if (row == 3)
    {
        // Capture to the left
        if (epRow == row - 1 &&
            epCol == col - 1)
        {
            moves.push_back(
                Move(row, col, epRow, epCol)
            );
        }

        // Capture to the right
        if (epRow == row - 1 &&
            epCol == col + 1)
        {
            moves.push_back(
                Move(row, col, epRow, epCol)
            );
        }
    }
}


}

    // Black pawn
    else if (piece == 'p')
    {
        // One square forward
       if (row + 1 <= 7 &&
    board.getPiece(row + 1, col) == '.')
{
    // Promotion
    if (row + 1 == 7)
    {
        moves.push_back(
            Move(row, col, row + 1, col, 'Q')
        );

        moves.push_back(
            Move(row, col, row + 1, col, 'R')
        );

        moves.push_back(
            Move(row, col, row + 1, col, 'B')
        );

        moves.push_back(
            Move(row, col, row + 1, col, 'N')
        );
    }
    else
    {
        // Normal move
        moves.push_back(
            Move(row, col, row + 1, col)
        );

        // Two squares from starting rank
        if (row == 1 &&
            board.getPiece(row + 2, col) == '.')
        {
            moves.push_back(
                Move(row, col, row + 2, col)
            );
        }
    }
}
       
if (row + 1 <= 7 && col - 1 >= 0 &&
    board.isWhitePiece(board.getPiece(row + 1, col - 1)))
{
    // Capture promotion
    if (row + 1 == 7)
    {
        moves.push_back(
            Move(row, col, row + 1, col - 1, 'Q')
        );

        moves.push_back(
            Move(row, col, row + 1, col - 1, 'R')
        );

        moves.push_back(
            Move(row, col, row + 1, col - 1, 'B')
        );

        moves.push_back(
            Move(row, col, row + 1, col - 1, 'N')
        );
    }
    else
    {
        moves.push_back(
            Move(row, col, row + 1, col - 1)
        );
    }
}

// Capture diagonally right
if (row + 1 <= 7 && col + 1 <= 7 &&
    board.isWhitePiece(board.getPiece(row + 1, col + 1)))
{
    // Capture promotion
    if (row + 1 == 7)
    {
        moves.push_back(
            Move(row, col, row + 1, col + 1, 'Q')
        );

        moves.push_back(
            Move(row, col, row + 1, col + 1, 'R')
        );

        moves.push_back(
            Move(row, col, row + 1, col + 1, 'B')
        );

        moves.push_back(
            Move(row, col, row + 1, col + 1, 'N')
        );
    }
    else
    {
        moves.push_back(
            Move(row, col, row + 1, col + 1)
        );
    }
}
    }
}

//Knight pseudo-move generator
void MoveGenerator::generateKnightMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves)
{
    char piece = board.getPiece(row, col);

    if (piece != 'N' && piece != 'n')
        return;

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

    for (int i = 0; i < 8; i++)
    {
        int newRow = row + knightMoves[i][0];
        int newCol = col + knightMoves[i][1];

        // Outside board
        if (newRow < 0 || newRow > 7 ||
            newCol < 0 || newCol > 7)
        {
            continue;
        }

        char destination = board.getPiece(newRow, newCol);

        // Cannot capture own piece
        if (piece == 'N' && board.isWhitePiece(destination))
            continue;

        if (piece == 'n' && board.isBlackPiece(destination))
            continue;

        moves.push_back(
            Move(row, col, newRow, newCol)
        );
    }

}

// Bishop + Queen diagonal pseudo-legal moves
void MoveGenerator::generateBishopMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves)
{
    char piece = board.getPiece(row, col);

    if (piece != 'B' && piece != 'b' &&
        piece != 'Q' && piece != 'q')
        return;

    const int directions[4][2] =
    {
        {-1, -1},
        {-1,  1},
        { 1, -1},
        { 1,  1}
    };

    for (int i = 0; i < 4; i++)
    {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];

        while (newRow >= 0 && newRow < 8 &&
               newCol >= 0 && newCol < 8)
        {
            char destination =
                board.getPiece(newRow, newCol);

            // Friendly piece blocks movement
            if ((piece == 'B' || piece == 'Q') &&
                board.isWhitePiece(destination))
                break;

            if ((piece == 'b' || piece == 'q') &&
                board.isBlackPiece(destination))
                break;

            // Empty square OR enemy piece
            moves.push_back(
                Move(row, col, newRow, newCol)
            );

            // Enemy piece was captured -> STOP
            if ((piece == 'B' || piece == 'Q') &&
                board.isBlackPiece(destination))
            {
                break;
            }

            if ((piece == 'b' || piece == 'q') &&
                board.isWhitePiece(destination))
            {
                break;
            }

            newRow += directions[i][0];
            newCol += directions[i][1];
        }
    }
}


// Rook + Queen straight pseudo-legal moves
void MoveGenerator::generateRookMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves)
{
    char piece = board.getPiece(row, col);

    if (piece != 'R' && piece != 'r' &&
        piece != 'Q' && piece != 'q')
        return;

    const int directions[4][2] =
    {
        {-1, 0},
        { 1, 0},
        { 0,-1},
        { 0, 1}
    };

    for (int i = 0; i < 4; i++)
    {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];

        while (newRow >= 0 && newRow < 8 &&
               newCol >= 0 && newCol < 8)
        {
            char destination =
                board.getPiece(newRow, newCol);

            // Friendly piece blocks movement
            if ((piece == 'R' || piece == 'Q') &&
                board.isWhitePiece(destination))
                break;

            if ((piece == 'r' || piece == 'q') &&
                board.isBlackPiece(destination))
                break;

            // Empty square OR enemy piece
            moves.push_back(
                Move(row, col, newRow, newCol)
            );

            // Enemy piece was captured -> STOP
            if ((piece == 'R' || piece == 'Q') &&
                board.isBlackPiece(destination))
            {
                break;
            }

            if ((piece == 'r' || piece == 'q') &&
                board.isWhitePiece(destination))
            {
                break;
            }

            newRow += directions[i][0];
            newCol += directions[i][1];
        }
    }
}
//Queen pseudo-legal moves
void MoveGenerator::generateQueenMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves)
{
    char piece = board.getPiece(row, col);

    if (piece != 'Q' && piece != 'q')
        return;

    generateBishopMoves(board, row, col, moves);
    generateRookMoves(board, row, col, moves);
}

//King pseudo-legal moves
void MoveGenerator::generateKingMoves(
    Board& board,
    int row,
    int col,
    std::vector<Move>& moves)
{
    char piece = board.getPiece(row, col);

    if (piece != 'K' && piece != 'k')
        return;

    const int directions[8][2] =
    {
        {-1, -1},
        {-1,  0},
        {-1,  1},
        { 0, -1},
        { 0,  1},
        { 1, -1},
        { 1,  0},
        { 1,  1}
    };

    for (int i = 0; i < 8; i++)
    {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];

        if (newRow < 0 || newRow > 7 ||
            newCol < 0 || newCol > 7)
        {
            continue;
        }

        char destination =
            board.getPiece(newRow, newCol);

        // Cannot capture own piece
        if (piece == 'K' && board.isWhitePiece(destination))
            continue;

        if (piece == 'k' && board.isBlackPiece(destination))
            continue;

        moves.push_back(
            Move(row, col, newRow, newCol)
        );
    }
}

