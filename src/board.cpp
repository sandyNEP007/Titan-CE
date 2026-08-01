#include <random>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "move.h"
#include "board.h"
#include "pieceSquareTables.h"


using namespace std;

Board::Board()
{
    initialize();
    setupPieces();
    initializeZobristTable();
    currentHash = generateHash();
    whiteKingMoved = false;
    blackKingMoved = false;

    whiteLeftRookMoved = false;
    whiteRightRookMoved = false;

    blackLeftRookMoved = false;
    blackRightRookMoved = false;
}

void Board::makeMove(const Move& move)
{
    
    char piece = board[move.fromRow][move.fromCol];

    
UndoInfo undo;

undo.move = move;

undo.movedPiece = board[move.fromRow][move.fromCol];
undo.capturedPiece = board[move.toRow][move.toCol];

undo.whiteKingMoved = whiteKingMoved;
undo.blackKingMoved = blackKingMoved;

undo.whiteLeftRookMoved = whiteLeftRookMoved;
undo.whiteRightRookMoved = whiteRightRookMoved;

undo.blackLeftRookMoved = blackLeftRookMoved;
undo.blackRightRookMoved = blackRightRookMoved;

    //--------------------------------

    // Update king movement flags
    if (piece == 'K')
        whiteKingMoved = true;

    if (piece == 'k')
        blackKingMoved = true;

    // Update rook movement flags
    if (piece == 'R' &&
        move.fromRow == 7 &&
        move.fromCol == 0)
    {
        whiteLeftRookMoved = true;
    }

    if (piece == 'R' &&
        move.fromRow == 7 &&
        move.fromCol == 7)
    {
        whiteRightRookMoved = true;
    }

    if (piece == 'r' &&
        move.fromRow == 0 &&
        move.fromCol == 0)
    {
        blackLeftRookMoved = true;
    }

    if (piece == 'r' &&
        move.fromRow == 0 &&
        move.fromCol == 7)
    {
        blackRightRookMoved = true;
    }

    // Castling: move the rook
    if (piece == 'K' || piece == 'k')
    {
        if (abs(move.toCol - move.fromCol) == 2)
        {
            // White kingside
            if (piece == 'K' &&
                move.fromRow == 7 && //here user input move for king where to row and col is 7,6
                move.fromCol == 4 &&
                move.toCol == 6)
            {
                board[7][5] = board[7][7];
                board[7][7] = '.';
            }

            // White queenside
            if (piece == 'K' &&
                move.fromRow == 7 &&
                move.fromCol == 4 &&
                move.toCol == 2)
            {
                board[7][3] = board[7][0];
                board[7][0] = '.';
            }

            // Black kingside
            if (piece == 'k' &&
                move.fromRow == 0 &&
                move.fromCol == 4 &&
                move.toCol == 6)
            {
                board[0][5] = board[0][7];
                board[0][7] = '.';
            }

            // Black queenside
            if (piece == 'k' &&
                move.fromRow == 0 &&
                move.fromCol == 4 &&
                move.toCol == 2)
            {
                board[0][3] = board[0][0];
                board[0][0] = '.';
            }
        }
    }

    // Move the piece
    board[move.toRow][move.toCol] = piece;
    board[move.fromRow][move.fromCol] = '.';
    

    // Handle pawn promotion
    pawnPromotion();

    undo.previousLastMove = lastMove;

    history.push_back(undo);

    lastMove = move;
}

bool Board::isWhitePiece(char piece)
{
    return piece >= 'A' && piece <= 'Z';
}

bool Board::isBlackPiece(char piece)
{
    return piece >= 'a' && piece <= 'z';
}

void Board::initialize()
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            board[row][col] = '.';
        }
    }
}

void Board::display()
{
    cout << "  a b c d e f g h" << endl;

    for (int row = 0; row < 8; row++)
    {
        cout << 8 - row << " ";

        for (int col = 0; col < 8; col++)
        {
            cout << board[row][col] << " ";
        }

        cout << endl;
    }
}

void Board::setupPieces()
{
    board[0][0]='r';
    board[0][1]='n';
    board[0][2]='b';
    board[0][3]='q';
    board[0][4]='k';
    board[0][5]='b';
    board[0][6]='n';
    board[0][7]='r';

    for(int col=0; col<8; col++)
        board[1][col]='p';

    for(int col=0; col<8; col++)
        board[6][col]='P';

    board[7][0]='R';
    board[7][1]='N';
    board[7][2]='B';
    board[7][3]='Q';
    board[7][4]='K';
    board[7][5]='B';
    board[7][6]='N';
    board[7][7]='R';
}

bool Board::isValidMove(const Move& move)
{
    if (move.fromRow < 0 || move.fromRow > 7 ||
        move.fromCol < 0 || move.fromCol > 7)
        return false;

    if (move.toRow < 0 || move.toRow > 7 ||
        move.toCol < 0 || move.toCol > 7)
        return false;

    if (move.fromRow == move.toRow &&
        move.fromCol == move.toCol)
        return false;

    if (board[move.fromRow][move.fromCol] == '.')
        return false;

    char sourcePiece = board[move.fromRow][move.fromCol];
    char destinationPiece = board[move.toRow][move.toCol];

    if (isWhitePiece(sourcePiece) && isWhitePiece(destinationPiece))
        return false;

    if (isBlackPiece(sourcePiece) && isBlackPiece(destinationPiece))
        return false;

    if (sourcePiece == 'P' || sourcePiece == 'p')
        return isValidPawnMove(move);

    if (sourcePiece == 'N' || sourcePiece == 'n')
        return isValidKnightMove(move);
        
    if (sourcePiece == 'B' || sourcePiece == 'b')
        return isValidBishopMove(move);

    if (sourcePiece == 'R' || sourcePiece == 'r')
        return isValidRookMove(move);

    if (sourcePiece == 'Q' || sourcePiece == 'q')
        return isValidQueenMove(move);

    if (sourcePiece == 'K' || sourcePiece == 'k')
        return isValidKingMove(move);

    return false;
}

bool Board::isValidKnightMove(const Move& move)
{
    int rowDifference = abs(move.toRow - move.fromRow);
    int colDifference = abs(move.toCol - move.fromCol);

    if ((rowDifference == 2 && colDifference == 1) ||
        (rowDifference == 1 && colDifference == 2))
    {
        return true;
    }

    return false;
}

bool Board::isValidPawnMove(const Move& move)
{
    char piece = board[move.fromRow][move.fromCol];
    char destinationPiece = board[move.toRow][move.toCol];

    if (piece == 'P')
    {
        // Two-square move
        if (move.fromRow == 6 &&
            move.toRow == 4 &&
            move.toCol == move.fromCol)
        {
            if (destinationPiece != '.')
                return false;

            if (board[move.fromRow - 1][move.fromCol] != '.')
                return false;

            return true;
        }

        // One-square move
        if (move.toRow == move.fromRow - 1 &&
            move.toCol == move.fromCol)
        {
            if (destinationPiece != '.')
                return false;

            return true;
        }
        // White diagonal capture
if (move.toRow == move.fromRow - 1 &&
    (move.toCol == move.fromCol - 1 ||
     move.toCol == move.fromCol + 1))
{
    if (isBlackPiece(destinationPiece))
    {
        return true;
    }

    return false;
}
// En passant
if (move.toRow == move.fromRow - 1 &&
    (move.toCol == move.fromCol - 1 ||
     move.toCol == move.fromCol + 1))
{
    if (destinationPiece == '.')
    {
        if (lastMove.fromRow == 1 &&
            lastMove.toRow == 3 &&
            lastMove.toCol == move.toCol)
        {
            if (board[move.fromRow][move.toCol] == 'p')
            {
                return true;
            }
        }
    }
}
}

    if (piece == 'p')
    {
        // Two-square move
        if (move.fromRow == 1 &&
            move.toRow == 3 &&
            move.toCol == move.fromCol)
        {
            if (destinationPiece != '.')
                return false;

            if (board[move.fromRow + 1][move.fromCol] != '.')
                return false;

            return true;
        }

        // One-square move
        if (move.toRow == move.fromRow + 1 &&
            move.toCol == move.fromCol)
        {
            if (destinationPiece != '.')
                return false;

            return true;
        }
        // Black diagonal capture
        // Black diagonal capture
if (move.toRow == move.fromRow + 1 &&
    (move.toCol == move.fromCol - 1 ||
     move.toCol == move.fromCol + 1))
{
    if (isWhitePiece(destinationPiece))
    {
        return true;
    }

    return false;
}
// En passant
if (move.toRow == move.fromRow + 1 &&
    (move.toCol == move.fromCol - 1 ||
     move.toCol == move.fromCol + 1))
{
    if (destinationPiece == '.')
    {
        if (lastMove.fromRow == 6 &&
            lastMove.toRow == 4 &&
            lastMove.toCol == move.toCol)
        {
            if (board[move.fromRow][move.toCol] == 'P')
            {
                return true;
            }
        }
    }
}
}

    return false;
}
bool Board::isValidBishopMove(const Move& move){
    int rowDifference = abs(move.toRow - move.fromRow);
    int colDifference = abs(move.toCol - move.fromCol);
 if (rowDifference != colDifference)
    {
        return false;
    }

    int rowStep = (move.toRow > move.fromRow) ? 1 : -1;
    int colStep = (move.toCol > move.fromCol) ? 1 : -1;

    int currentRow = move.fromRow + rowStep;
    int currentCol = move.fromCol + colStep;
while (currentRow != move.toRow)
{
    if (board[currentRow][currentCol] != '.')
    {
        return false;
    }

    currentRow += rowStep;
    currentCol += colStep;
}

return true;
}
bool Board::isValidRookMove(const Move& move){
    if (move.fromRow != move.toRow &&
        move.fromCol != move.toCol)
    {
        return false;
    }

    int rowStep = 0;
    int colStep = 0;

    if (move.toRow > move.fromRow)
        rowStep = 1;
    else if (move.toRow < move.fromRow)
        rowStep = -1;

    if (move.toCol > move.fromCol)
        colStep = 1;
    else if (move.toCol < move.fromCol)
        colStep = -1;

    int currentRow = move.fromRow + rowStep;
    int currentCol = move.fromCol + colStep;
    while (currentRow != move.toRow ||
       currentCol != move.toCol)
{
    if (board[currentRow][currentCol] != '.')
    {
        return false;
    }

    currentRow += rowStep;
    currentCol += colStep;
}

return true;
}
bool Board::isValidQueenMove(const Move& move)
{
    if (isValidBishopMove(move))
    {
        return true;
    }

    if (isValidRookMove(move))
    {
        return true;
    }

    return false;
}
bool Board::isValidKingMove(const Move& move)
{
    int rowDifference = abs(move.toRow - move.fromRow);
    int colDifference = abs(move.toCol - move.fromCol);

    if (rowDifference == 0 && colDifference == 0)
    {
        return false;
    }
    //---------------
     if (rowDifference == 0 && colDifference == 2)
{
    char piece = board[move.fromRow][move.fromCol];

     if (piece == 'K' && whiteKingMoved)
    {
    return false;
    }

    if (piece == 'k' && blackKingMoved)
   {
    return false;
   }
   // White kingside
if (piece == 'K' && move.toCol == 6)
{
    if (board[7][5] != '.' || board[7][6] != '.')
    {
        return false;
    }
}

// White queenside
if (piece == 'K' && move.toCol == 2)
{
    if (board[7][1] != '.' ||
        board[7][2] != '.' ||
        board[7][3] != '.')
    {
        return false;
    }
}

// Black kingside
if (piece == 'k' && move.toCol == 6)
{
    if (board[0][5] != '.' || board[0][6] != '.')
    {
        return false;
    }
}

// Black queenside
if (piece == 'k' && move.toCol == 2)
{
    if (board[0][1] != '.' ||
        board[0][2] != '.' ||
        board[0][3] != '.')
    {
        return false;
    }
}
if (piece == 'K')
{
    if (isKingInCheck(true))
    {
        return false;
    }
}

if (piece == 'k')
{
    if (isKingInCheck(false))
    {
        return false;
    }
}// White kingside: e1 -> f1
if (piece == 'K' && move.toCol == 6)
{
    board[7][4] = '.';
    board[7][5] = 'K';

    bool inCheck = isKingInCheck(true);

    board[7][5] = '.';
    board[7][4] = 'K';

    if (inCheck)
    {
        return false;
    }
}
if (piece == 'K' && move.toCol == 2)
{
    board[7][4] = '.';
    board[7][3] = 'K';

    bool inCheck = isKingInCheck(true);

    board[7][3] = '.';
    board[7][4] = 'K';

    if (inCheck)
    {
        return false;
    }
}if (piece == 'k' && move.toCol == 6)
{
    board[0][4] = '.';
    board[0][5] = 'k';

    bool inCheck = isKingInCheck(false);

    board[0][5] = '.';
    board[0][4] = 'k';

    if (inCheck)
    {
        return false;
    }
}
   // White kingside
if (piece == 'K' && move.toCol == 6 && whiteRightRookMoved)
{
    return false;
}

// White queenside
if (piece == 'K' && move.toCol == 2 && whiteLeftRookMoved)
{
    return false;
}

// Black kingside
if (piece == 'k' && move.toCol == 6 && blackRightRookMoved)
{
    return false;
}

// Black queenside
if (piece == 'k' && move.toCol == 2 && blackLeftRookMoved)
{
    return false;
}
if (piece == 'K')
{
    board[7][4] = '.';
    board[move.toRow][move.toCol] = 'K';

    bool inCheck = isKingInCheck(true);

    board[move.toRow][move.toCol] = '.';
    board[7][4] = 'K';

    if (inCheck)
    {
        return false;
    }
}
if (piece == 'k')
{
    board[0][4] = '.';
    board[move.toRow][move.toCol] = 'k';

    bool inCheck = isKingInCheck(false);

    board[move.toRow][move.toCol] = '.';
    board[0][4] = 'k';

    if (inCheck)
    {
        return false;
    }
}
return true;
}
    //------------------

    if (rowDifference <= 1 && colDifference <= 1)
    {
        return true;
    }

    return false;
}


bool Board::isLegalMove(const Move& move){
    if (!isValidMove(move))
    {
        return false;
    }

    char sourcePiece = board[move.fromRow][move.fromCol];
    char destinationPiece = board[move.toRow][move.toCol];

    // Make move temporarily
    makeMove(move);
    

    bool inCheck;

    if (isWhitePiece(sourcePiece))
    {
        inCheck = isKingInCheck(true);
    }
    else
    {
        inCheck = isKingInCheck(false);
    }

    // Undo move
    undoMove(move);
    

    return !inCheck;
}
bool Board::hasLegalMove(bool whitePlayer)
{
    
    for (int fromRow = 0; fromRow < 8; fromRow++)
    {
        for (int fromCol = 0; fromCol < 8; fromCol++)
        {
            char piece = board[fromRow][fromCol];

            if ((whitePlayer && !isWhitePiece(piece)) ||
                (!whitePlayer && !isBlackPiece(piece)))
            {
                continue;
            }

            for (int toRow = 0; toRow < 8; toRow++)
            {
                for (int toCol = 0; toCol < 8; toCol++)
                {
                    Move move(fromRow, fromCol, toRow, toCol);

                    if (isLegalMove(move))
                    return true;
                    
                }
            }
        }
    }
    return false;
}



int Board::BoardEvaluation()
{
    int score = 0;
    int blackBishops = 0;
    int whiteBishops = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            
            

            switch (board[row][col])
            {
                case 'P': 
                score += 100; 
                score +=PAWN_PST[row][col];
                break;
                //------------
                case 'N': 
                score += 320; 
                score +=KNIGHT_PST[row][col];
                break;
                //-------------
                case 'B': 
                whiteBishops++;
                score += 330; 
                score +=BISHOP_PST[row][col];
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
                score += KING_PST[row][col];
                break;
                //------------
                case 'p': 
                score -= 100; 
                score -=PAWN_PST[7-row][col];
                break;
                //------------
                case 'n': 
                score -= 320; 
                score -=KNIGHT_PST[7-row][col];
                break;
                //-------------
                case 'b': 
                blackBishops++;
                score -= 330; 
                score -=BISHOP_PST[7-row][col];
                break;
                case 'r': 
                score -= 500; 
                score -= ROOK_PST[7-row][col];
                break;
                case 'q': 
                score -= 900; 
                score -= QUEEN_PST[7-row][col];
                break;
                case 'k':
                score -= KING_PST[7-row][col];
                break;
            }
        }
    }
    if (whiteBishops >= 2)
    score += 30;

    if (blackBishops >= 2)
    score -= 30;

    return score;
}
std::vector<Move> Board::generateLegalMoves(bool whitePlayer)
{
    std::vector<Move> legalMoves;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board[row][col];

            if (whitePlayer)
            {
                if (!isWhitePiece(piece))
                {
                    continue;
                }
            }
            else
            {
                if (!isBlackPiece(piece))
                {
                    continue;
                }
            }

            for (int toRow = 0; toRow < 8; toRow++)
{
                   for (int toCol = 0; toCol < 8; toCol++)
    {
        Move move(row, col, toRow, toCol);

        if (isLegalMove(move))
        {
            legalMoves.push_back(move);
        }
    }
}

        }
    }

    return legalMoves;
}




void Board::undoMove(const Move& move)
{
    UndoInfo undo = history.back();

    // Undo castling rook move
    if ((undo.movedPiece == 'K' || undo.movedPiece == 'k') &&
        abs(undo.move.toCol - undo.move.fromCol) == 2)
    {
        // White kingside
        if (undo.movedPiece == 'K' &&
            undo.move.fromRow == 7 &&
            undo.move.toCol == 6)
        {
            board[7][7] = board[7][5];
            board[7][5] = '.';
        }

        // White queenside
        else if (undo.movedPiece == 'K' &&
                 undo.move.fromRow == 7 &&
                 undo.move.toCol == 2)
        {
            board[7][0] = board[7][3];
            board[7][3] = '.';
        }

        // Black kingside
        else if (undo.movedPiece == 'k' &&
                 undo.move.fromRow == 0 &&
                 undo.move.toCol == 6)
        {
            board[0][7] = board[0][5];
            board[0][5] = '.';
        }

        // Black queenside
        else if (undo.movedPiece == 'k' &&
                 undo.move.fromRow == 0 &&
                 undo.move.toCol == 2)
        {
            board[0][0] = board[0][3];
            board[0][3] = '.';
        }
    }

    // Existing code
    board[undo.move.fromRow][undo.move.fromCol] = undo.movedPiece;
    board[undo.move.toRow][undo.move.toCol] = undo.capturedPiece;

    whiteKingMoved = undo.whiteKingMoved;
    blackKingMoved = undo.blackKingMoved;

    whiteLeftRookMoved = undo.whiteLeftRookMoved;
    whiteRightRookMoved = undo.whiteRightRookMoved;

    blackLeftRookMoved = undo.blackLeftRookMoved;
    blackRightRookMoved = undo.blackRightRookMoved;

    history.pop_back();
    lastMove = undo.previousLastMove;
}

void Board::initializeZobristTable()
{
    std::mt19937_64 rng(2026);

    for (int piece = 0; piece < 12; piece++)
    {
        for (int square = 0; square < 64; square++)
        {
            zobristTable[piece][square] = rng();
        }
    }
}
int Board::pieceToIndex(char piece)
{
    if (piece == 'P')
        return 0;
    else if (piece == 'N')
        return 1;
    else if (piece == 'B')
        return 2;
    else if (piece == 'R')
        return 3;
    else if (piece == 'Q')
        return 4;
    else if (piece == 'K')
        return 5;

    else if (piece == 'p')
        return 6;
    else if (piece == 'n')
        return 7;
    else if (piece == 'b')
        return 8;
    else if (piece == 'r')
        return 9;
    else if (piece == 'q')
        return 10;
    else if (piece == 'k')
        return 11;

    return -1;
}
uint64_t Board::generateHash()
{
    uint64_t hash = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board[row][col];

            int pieceIndex = pieceToIndex(piece);

            if (pieceIndex == -1)
            {
                continue;
            }

            int square = row * 8 + col;

            hash ^= zobristTable[pieceIndex][square];
        }
    }

    return hash;
}
void Board::debugSearch(const Move& move,
                        int depth,
                        int score,
                        const std::string& action)
{
    

    

    cout << " "
         << char('a' + move.fromCol)
         << 8 - move.fromRow
         << " -> "
         << char('a' + move.toCol)
         << 8 - move.toRow
         << endl;

   

    
}
bool Board::isKingInCheck(bool whiteKing)
{
    char king = whiteKing ? 'K' : 'k';

    int kingRow = -1;
    int kingCol = -1;

    // Find the king
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            if (board[row][col] == king)
            {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }
         return isSquareAttacked(kingRow, kingCol, !whiteKing);
}


bool Board::isSquareAttacked(int row, int col, bool byWhite){
    if (byWhite)
{
    if (row > 0)
    {
        if (col > 0 && board[row - 1][col - 1] == 'P')
            return true;

        if (col < 7 && board[row - 1][col + 1] == 'P')
            return true;
    }
}
else
{
    if (row < 7)
    {
        if (col > 0 && board[row + 1][col - 1] == 'p')
            return true;

        if (col < 7 && board[row + 1][col + 1] == 'p')
            return true;
    }
}
const int knightMoves[8][2] =
{
    {-2,-1},
    {-2, 1},
    {-1,-2},
    {-1, 2},
    { 1,-2},
    { 1, 2},
    { 2,-1},
    { 2, 1}
};

for (int i = 0; i < 8; i++)
{
    int r = row + knightMoves[i][0];
    int c = col + knightMoves[i][1];

    if (r < 0 || r > 7 || c < 0 || c > 7)
        continue;

    if (byWhite)
    {
        if (board[r][c] == 'N')
            return true;
    }
    else
    {
        if (board[r][c] == 'n')
            return true;
    }
}
// Bishop directions
const int bishopDirections[4][2] =
{
    {-1,-1},
    {-1, 1},
    { 1,-1},
    { 1, 1}
};

for (int d = 0; d < 4; d++)
{
    int r = row + bishopDirections[d][0];
    int c = col + bishopDirections[d][1];

    while (r >= 0 && r < 8 && c >= 0 && c < 8)
    {
        if (board[r][c] != '.')
        {
            if (byWhite)
            {
                if (board[r][c] == 'B' || board[r][c] == 'Q')
                    return true;
            }
            else
            {
                if (board[r][c] == 'b' || board[r][c] == 'q')
                    return true;
            }

            break;      // blocked by any piece
        }

        r += bishopDirections[d][0];
        c += bishopDirections[d][1];
    }
}
// Rook directions
const int rookDirections[4][2] =
{
    {-1, 0}, // Up
    { 1, 0}, // Down
    { 0,-1}, // Left
    { 0, 1}  // Right
};

for (int d = 0; d < 4; d++)
{
    int r = row + rookDirections[d][0];
    int c = col + rookDirections[d][1];

    while (r >= 0 && r < 8 && c >= 0 && c < 8)
    {
        if (board[r][c] != '.')
        {
            if (byWhite)
            {
                if (board[r][c] == 'R' || board[r][c] == 'Q')
                    return true;
            }
            else
            {
                if (board[r][c] == 'r' || board[r][c] == 'q')
                    return true;
            }

            break;      // blocked
        }

        r += rookDirections[d][0];
        c += rookDirections[d][1];
    }
}
// King attacks
const int kingMoves[8][2] =
{
    {-1,-1},
    {-1, 0},
    {-1, 1},
    { 0,-1},
    { 0, 1},
    { 1,-1},
    { 1, 0},
    { 1, 1}
};

for (int i = 0; i < 8; i++)
{
    int r = row + kingMoves[i][0];
    int c = col + kingMoves[i][1];

    if (r < 0 || r > 7 || c < 0 || c > 7)
        continue;

    if (byWhite)
    {
        if (board[r][c] == 'K')
            return true;
    }
    else
    {
        if (board[r][c] == 'k')
            return true;
    }
}

return false;
}

    bool Board::isCheckmate(bool whitePlayer)
{
    return isKingInCheck(whitePlayer) &&
           !hasLegalMove(whitePlayer);
}

bool Board::isStalemate(bool whitePlayer)
{
    return !isKingInCheck(whitePlayer) &&
           !hasLegalMove(whitePlayer);
}
void Board::pawnPromotion()
{
    // Temporary placeholder
}

int Board :: moveScore(const Move& move){
 int score = 0;

    char attacker = board[move.fromRow][move.fromCol];
    char victim   = board[move.toRow][move.toCol];
    

    switch (victim)
    {
        case 'Q':
        case 'q': score += 900; break;

        case 'R':
        case 'r': score += 500; break;

        case 'B':
        case 'b':
        case 'N':
        case 'n': score += 300; break;

        case 'P':
        case 'p': score += 100; break;
    }

    return score;

}