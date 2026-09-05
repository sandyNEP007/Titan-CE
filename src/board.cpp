#include<iostream>
#include<string>
#include "board.h"
#include "zobrist.h"
#include "move.h"
#include <cstdlib>
using namespace std;

 static void removePieceHash(uint64_t& hash, char piece, int row, int col)
{
    if (piece == '.')
        return;

    int square = row * 8 + col;

    hash ^= Zobrist::getPieceKey(piece, square);
}
int Board::pieceToIndex(char piece)
{
    switch (piece)
    {
        case 'P': return WP;
        case 'N': return WN;
        case 'B': return WB;
        case 'R': return WR;
        case 'Q': return WQ;
        case 'K': return WK;
        case 'p': return BP;
        case 'n': return BN;
        case 'b': return BB;
        case 'r': return BR;
        case 'q': return BQ;
        case 'k': return BK;
    }
    return -1;
}

void Board::syncBitboards()
{
    for (int i = 0; i < 12; i++)
        pieceBB[i] = 0ULL;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board[row][col];
            int idx = pieceToIndex(piece);

            if (idx != -1)
            {
                int square = row * 8 + col;
                pieceBB[idx] |= (1ULL << square);
            }
        }
    }

    whiteOccupancy = pieceBB[WP] | pieceBB[WN] | pieceBB[WB] |
                      pieceBB[WR] | pieceBB[WQ] | pieceBB[WK];

    blackOccupancy = pieceBB[BP] | pieceBB[BN] | pieceBB[BB] |
                      pieceBB[BR] | pieceBB[BQ] | pieceBB[BK];

    allOccupancy = whiteOccupancy | blackOccupancy;
}
uint64_t Board::knightAttacks[64];
uint64_t Board::kingAttacks[64];
uint64_t Board::pawnAttacks[2][64];
bool Board::attackTablesInitialized = false;

void Board::initAttackTables()
{
    if (attackTablesInitialized)
        return;

    for (int square = 0; square < 64; square++)
    {
        int row = square / 8;
        int col = square % 8;

        // Knight
        uint64_t knightBB = 0ULL;
        const int knightOffsets[8][2] = {
            {-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}
        };
        for (auto& off : knightOffsets)
        {
            int r = row + off[0], c = col + off[1];
            if (r >= 0 && r < 8 && c >= 0 && c < 8)
                knightBB |= (1ULL << (r * 8 + c));
        }
        knightAttacks[square] = knightBB;

        // King
        uint64_t kingBB = 0ULL;
        for (int dr = -1; dr <= 1; dr++)
        {
            for (int dc = -1; dc <= 1; dc++)
            {
                if (dr == 0 && dc == 0)
                    continue;

                int r = row + dr, c = col + dc;
                if (r >= 0 && r < 8 && c >= 0 && c < 8)
                    kingBB |= (1ULL << (r * 8 + c));
            }
        }
        kingAttacks[square] = kingBB;

        // Pawns — white pawns attack toward row+1 (matches the existing
        // isSquareAttacked logic: "white pawns attack upward" = row+1).
        uint64_t whitePawnBB = 0ULL;
        if (row + 1 < 8)
        {
            if (col - 1 >= 0) whitePawnBB |= (1ULL << ((row + 1) * 8 + col - 1));
            if (col + 1 < 8)  whitePawnBB |= (1ULL << ((row + 1) * 8 + col + 1));
        }
        pawnAttacks[0][square] = whitePawnBB;

        uint64_t blackPawnBB = 0ULL;
        if (row - 1 >= 0)
        {
            if (col - 1 >= 0) blackPawnBB |= (1ULL << ((row - 1) * 8 + col - 1));
            if (col + 1 < 8)  blackPawnBB |= (1ULL << ((row - 1) * 8 + col + 1));
        }
        pawnAttacks[1][square] = blackPawnBB;
    }

    attackTablesInitialized = true;
}

Board :: Board(){    
    Zobrist::initialize();
    Board::initAttackTables();
    BoardInitialize();
    setPieces();
    whiteTurn = true;

    whiteKingMoved = false;
    blackKingMoved = false;

    whiteLeftRookMoved = false;
    whiteRightRookMoved = false;

    blackLeftRookMoved = false;
    blackRightRookMoved = false;
    whiteHasCastle = false;
    blackHasCastle = false;

    zobristHash = Zobrist::generateHash(*this);
    syncBitboards();
    
}

//Initializaton of chess Board
void Board :: BoardInitialize()
{
    for(int i=0; i<8; i++){
        for(int j =0; j<8; j++){
            board[i][j] = '.';
        }
    }

}

//Setting pieces on board
void Board :: setPieces(){
    //Setting white pieces on 2nd rank
    for(int i=0; i<8; i++){
        board[6][i] = 'P';
    }
    //Setting black pieces on 7th rank
    for(int i=0; i<8; i++){
        board[1][i] = 'p';
    }
    //for all White pieces
    board[7][0] = 'R';
    board[7][1] = 'N';
    board[7][2] = 'B';
    board[7][3] = 'Q';
    board[7][4] = 'K';
    board[7][5] = 'B';
    board[7][6] = 'N';
    board[7][7] = 'R';
    //for all Black pieces
    board[0][0] = 'r';
    board[0][1] = 'n';
    board[0][2] = 'b';
    board[0][3] = 'q';
    board[0][4] = 'k';
    board[0][5] = 'b';
    board[0][6] = 'n';
    board[0][7] = 'r';
}

//Diaplaying board
void Board :: display(){
      for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            cout<<board[i][j]<<" ";
        }
        cout<<endl;
    }
   cout<<"a b c d e f g h"<<endl;
}

//Checking if the piece is white or black
bool Board :: isWhitePiece(char piece)
{
    return piece >= 'A' && piece <= 'Z';
}
bool Board :: isBlackPiece(char piece)
{
     return piece >= 'a' && piece <= 'z';
}

//Validating move
bool Board::isValidMove(const Move& move)
{

    //Check coordinates
    if (move.fromRow < 0 || move.fromRow > 7 ||
        move.fromCol < 0 || move.fromCol > 7 ||
        move.toRow < 0 || move.toRow > 7 ||
        move.toCol < 0 || move.toCol > 7)
    {
        return false;
    }

    if (move.fromRow == move.toRow &&
        move.fromCol == move.toCol)
    {
        return false;
    }

    char sourcePiece = board[move.fromRow][move.fromCol];
    char destinationPiece = board[move.toRow][move.toCol];
    bool movingWhite = isWhitePiece(sourcePiece);
       
    if (sourcePiece == '.')
    {
        return false;
    }

    //Cannot capture your own piece
    if (isWhitePiece(sourcePiece) &&
        isWhitePiece(destinationPiece))
    {
        return false;
    }

    if (isBlackPiece(sourcePiece) &&
        isBlackPiece(destinationPiece))
    {
        return false;
    }

    bool pieceMoveValid = false;

    if (sourcePiece == 'P' || sourcePiece == 'p')
        pieceMoveValid = isValidPawnMove(move);

    else if (sourcePiece == 'N' || sourcePiece == 'n')
        pieceMoveValid = isValidKnightMove(move);

    else if (sourcePiece == 'B' || sourcePiece == 'b')
        pieceMoveValid = isValidBishopMove(move);

    else if (sourcePiece == 'R' || sourcePiece == 'r')
        pieceMoveValid = isValidRookMove(move);

    else if (sourcePiece == 'Q' || sourcePiece == 'q')
        pieceMoveValid = isValidQueenMove(move);

    else if (sourcePiece == 'K' || sourcePiece == 'k')
{

    // Castling attempt: king moves exactly two files
    if (move.fromRow == move.toRow &&
        abs(move.toCol - move.fromCol) == 2)
    {
        pieceMoveValid = isValidCastle(move);
    }
    else
    {
        pieceMoveValid = isValidKingMove(move);
    }
}

    else
        return false;

    //Piece cannot move that way
    if (!pieceMoveValid)
    {
        return false;
    }

    // Temporarily make the move
makeMove(move);

// Check whether the moving side's king is now in check
bool illegal = isKingInCheck(movingWhite);

// Restore the board
undoMove();

// If our king is exposed, move is illegal
if (illegal)
{
    return false;
}

return true;
}


bool Board :: isValidPawnMove(const Move& move)
{
     char piece = board[move.fromRow][move.fromCol];
     char destPiece = board[move.toRow][move.toCol];
     //for white pawn
     if(piece == 'P')
{
     if (move.fromRow == 6 &&        //two square move
         move.toRow == 4 &&
         move.toCol == move.fromCol)
        {
            if (destPiece != '.')
                return false;

            if (board[move.fromRow - 1][move.fromCol] != '.')
                return false;

            return true;
        }

         // One-square move
        if (move.toRow == move.fromRow - 1 &&
            move.toCol == move.fromCol)
        {
            if (destPiece != '.')
                return false;

            return true;
        }

         // White diagonal capture
         if (move.toRow == move.fromRow - 1 &&
            (move.toCol == move.fromCol - 1 ||
            move.toCol == move.fromCol + 1))
         {
         if (isBlackPiece(destPiece))
         {
            return true;
         }

           return false;
    
        }
        //En-passant
        if (move.toRow == move.fromRow - 1 &&
           (move.toCol == move.fromCol - 1 ||
            move.toCol == move.fromCol + 1))
{
    if (destPiece == '.')
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

if(piece=='p')
{
         // Two-square move
        if (move.fromRow == 1 &&
            move.toRow == 3 &&
            move.toCol == move.fromCol)
        {
            if (destPiece != '.')
                return false;

            if (board[move.fromRow + 1][move.fromCol] != '.')
                return false;

            return true;
        }

        // One-square move
        if (move.toRow == move.fromRow + 1 &&
            move.toCol == move.fromCol)
        {
            if (destPiece != '.')
                return false;

            return true;
        }
        // Black diagonal capture
       if (move.toRow == move.fromRow + 1 &&
          (move.toCol == move.fromCol - 1 ||
          move.toCol == move.fromCol + 1))
        {
            if (isWhitePiece(destPiece))
            {
               return true;
            }

          return false;
         }
          //En-passant
          if (move.toRow == move.fromRow + 1 &&
    (move.toCol == move.fromCol - 1 ||
     move.toCol == move.fromCol + 1))
{
    if (destPiece == '.')
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
    char piece = board[move.fromRow][move.fromCol];
    char destPiece = board[move.toRow][move.toCol];

    if (abs(move.toCol - move.fromCol) == 2)
        {
           return isValidCastle(move);
        }

    if (piece != 'K' && piece != 'k')
        {
           return false;
        }

    
    if (abs(move.toRow - move.fromRow) > 1 ||
        abs(move.toCol - move.fromCol) > 1)
    {
        return false;
    }

    // Cannot capture own piece
    if (isWhitePiece(piece) && isWhitePiece(destPiece))
    {
        return false;
    }

    if (isBlackPiece(piece) && isBlackPiece(destPiece))
    {
        return false;
    }

    bool white = (piece == 'K');

    // Check destination square
    if (isSquareAttacked(move.toRow, move.toCol, !white))
    {
        return false;
    }

    return true;
}

bool Board::isKingInCheck(bool white)
{
    char king = white ? 'K' : 'k';

    int kingRow = -1;
    int kingCol = -1;

    // Find king
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

    // Opposite color attacks this king
    return isSquareAttacked(kingRow, kingCol, !white);
}

bool Board :: isSquareAttacked(int row, int col, bool byWhite)
{   
    //Pawn attack
     //Pawn attack — O(1) bitboard lookup instead of manual offset checks.
    int square = row * 8 + col;

    if (byWhite)
    {
        if (pawnAttacks[0][square] & pieceBB[WP])
            return true;
    }
    else
    {
        if (pawnAttacks[1][square] & pieceBB[BP])
            return true;
    }

    // Knight attacks
        // Knight attacks — O(1) bitboard lookup instead of an 8-way loop.
    int targetSquare = row * 8 + col;
    uint64_t enemyKnights = byWhite ? pieceBB[WN] : pieceBB[BN];

    if (knightAttacks[targetSquare] & enemyKnights)
        return true;

    // King attacks — O(1) bitboard lookup instead of a 3x3 loop.
    uint64_t enemyKingBB = byWhite ? pieceBB[WK] : pieceBB[BK];

    if (kingAttacks[targetSquare] & enemyKingBB)
        return true;

//Bishop attack
char bishop = byWhite ? 'B' : 'b';
char queen  = byWhite ? 'Q' : 'q';

const int diagonalDirections[4][2] =
{
    {-1, -1},   
    {-1,  1},   
    { 1, -1},   
    { 1,  1}    
};

for (int i = 0; i < 4; i++)
{
    int r = row + diagonalDirections[i][0];
    int c = col + diagonalDirections[i][1];

    while (r >= 0 && r < 8 &&
           c >= 0 && c < 8)
    {
        
        if (board[r][c] != '.')
        {
            // Enemy bishop or queen found
            if (board[r][c] == bishop ||
                board[r][c] == queen)
            {
                return true;
            }

            break;
        }

        // Continue along the diagonal
        r += diagonalDirections[i][0];
        c += diagonalDirections[i][1];
    }
}
 
// Rook / Queen attacks
char rook = byWhite ? 'R' : 'r';
const int straightDirections[4][2] =
{
    {-1, 0},    
    { 1, 0},    
    { 0,-1},    
    { 0, 1}     
};

for (int i = 0; i < 4; i++)
{
    int r = row + straightDirections[i][0];
    int c = col + straightDirections[i][1];

    while (r >= 0 && r < 8 &&
           c >= 0 && c < 8)
    {
        if (board[r][c] != '.')
        {
            // Rook or Queen found
            if (board[r][c] == rook ||
                board[r][c] == queen)
            {
                return true;
            }

            break;
        }

        r += straightDirections[i][0];
        c += straightDirections[i][1];
    }
}
    return false;
}

static int getCastlingRights(const Board& board)
{
    int rights = 0;

    if (board.canWhiteKingSideCastle())
        rights |= 1;

    if (board.canWhiteQueenSideCastle())
        rights |= 2;

    if (board.canBlackKingSideCastle())
        rights |= 4;

    if (board.canBlackQueenSideCastle())
        rights |= 8;

    return rights;
}

//After validating move board make move
void Board::makeMove(const Move& move)
{
    UndoInfo undo;

    undo.move = move;
    undo.previousWhiteTurn = whiteTurn;
    undo.previouslastMove = lastMove;

    undo.movedPiece =
        board[move.fromRow][move.fromCol];

    undo.capturedPiece =
        board[move.toRow][move.toCol];

    undo.whiteKingMoved = whiteKingMoved;
    undo.blackKingMoved = blackKingMoved;

    undo.whiteLeftRookMoved = whiteLeftRookMoved;

    undo.whiteRightRookMoved = whiteRightRookMoved;

    undo.blackLeftRookMoved = blackLeftRookMoved;

    undo.blackRightRookMoved = blackRightRookMoved;
    undo.previousWhiteHasCastled = whiteHasCastle;
    undo.previousBlackHasCastled = blackHasCastle;

    undo.wasCastling = false;
    undo.wasEnPassant = false;
    undo.enPassantCapturedPiece = '.';
    undo.enPassantCapturedRow = -1;
    undo.enPassantCapturedCol = -1;

    // Save current hash
    undo.previousZobristHash =
        zobristHash;

    history.push_back(undo);

    int oldCastlingRights = getCastlingRights(*this);

    int oldEpSquare = getEnPassantSquare();

    // Remove old castling rights
    zobristHash ^=
        Zobrist::getCastlingKey(oldCastlingRights);

    // Remove old en-passant state
    if (oldEpSquare != -1)
    {
        zobristHash ^= Zobrist::getEnPassantKey(
                oldEpSquare);
    }

    // Remove old side-to-move
    zobristHash ^= Zobrist::getSideKey();

    char movedPiece = board[move.fromRow][move.fromCol];

    bool isCastling = false;
    bool isEnPassant = false;

    if (movedPiece == 'K' &&
        move.fromRow == 7 &&
        move.fromCol == 4 &&
        move.toRow == 7 &&
        (move.toCol == 6 ||
         move.toCol == 2))
    {
        isCastling = true;
    }

    if (movedPiece == 'k' &&
        move.fromRow == 0 &&
        move.fromCol == 4 &&
        move.toRow == 0 &&
        (move.toCol == 6 ||
         move.toCol == 2))
    {
        isCastling = true;
    }

    // Normal moving piece
    removePieceHash(
        zobristHash,
        movedPiece,
        move.fromRow,
        move.fromCol);

    // Captured piece
    removePieceHash(
        zobristHash,
        undo.capturedPiece,
        move.toRow,
        move.toCol);

  int enPassantSquare = getEnPassantSquare();

if (enPassantSquare != -1 &&
    (movedPiece == 'P' || movedPiece == 'p'))
{
    int epRow = enPassantSquare / 8;
    int epCol = enPassantSquare % 8;

    if (move.toRow == epRow &&
        move.toCol == epCol &&
        board[move.toRow][move.toCol] == '.')
    {
        isEnPassant = true;
    }
}
history.back().wasCastling = isCastling;

   if (isEnPassant)
{
    int capturedPawnRow;

    if (movedPiece == 'P')
        capturedPawnRow = move.toRow + 1;
    else
        capturedPawnRow = move.toRow - 1;

    char capturedPawn =
        board[capturedPawnRow][move.toCol];

    // Save EP information for undo
    history.back().wasEnPassant = true;

    history.back().enPassantCapturedPiece =
        capturedPawn;

    history.back().enPassantCapturedRow =
        capturedPawnRow;

    history.back().enPassantCapturedCol =
        move.toCol;

    // Remove captured pawn from Zobrist hash
    removePieceHash(
        zobristHash,
        capturedPawn,
        capturedPawnRow,
        move.toCol);

    // Remove captured pawn from board
    board[capturedPawnRow][move.toCol] = '.';
}
    if (isCastling)
    {

        if (movedPiece == 'K' &&
            move.toCol == 6)
        {
            // Remove rook from old square
            removePieceHash(zobristHash,'R',7,7);

            // Add king to g1
            zobristHash ^=Zobrist::getPieceKey('K',7 * 8 + 6);

            // Add rook to f1
            zobristHash ^= Zobrist::getPieceKey('R', 7 * 8 + 5);

            board[7][4] = '.';
            board[7][6] = 'K';

            board[7][7] = '.';
            board[7][5] = 'R';

            whiteKingMoved = true;
            whiteRightRookMoved = true;
            whiteHasCastle = true;
        }

        else if (movedPiece == 'K' &&
                 move.toCol == 2)
        {
            removePieceHash(zobristHash,'R',7,0);

            zobristHash ^=Zobrist::getPieceKey('K',7 * 8 + 2);

            zobristHash ^=Zobrist::getPieceKey('R', 7 * 8 + 3);

            board[7][4] = '.';
            board[7][2] = 'K';

            board[7][0] = '.';
            board[7][3] = 'R';

            whiteKingMoved = true;
            whiteLeftRookMoved = true;
            whiteHasCastle = true;
        }

       

        else if (movedPiece == 'k' &&
                 move.toCol == 6)
        {
            removePieceHash(
                zobristHash,
                'r',
                0,
                7);

            zobristHash ^=
                Zobrist::getPieceKey(
                    'k',
                    0 * 8 + 6);

            zobristHash ^=
                Zobrist::getPieceKey(
                    'r',
                    0 * 8 + 5);

            board[0][4] = '.';
            board[0][6] = 'k';

            board[0][7] = '.';
            board[0][5] = 'r';

            blackKingMoved = true;
            blackRightRookMoved = true;
            blackHasCastle = true;
        }

       
        else
        {
            removePieceHash(zobristHash, 'r', 0, 0);

            zobristHash ^= Zobrist::getPieceKey('k', 0 * 8 + 2);

            zobristHash ^= Zobrist::getPieceKey('r',0 * 8 + 3);

            board[0][4] = '.';
            board[0][2] = 'k';

            board[0][0] = '.';
            board[0][3] = 'r';

            blackKingMoved = true;
            blackLeftRookMoved = true;
            blackHasCastle = true;
        }

        lastMove = move;
        whiteTurn = !whiteTurn;
    }

    else
    {
        // Update castling rights
        if (movedPiece == 'K')
            whiteKingMoved = true;

        if (movedPiece == 'k')
            blackKingMoved = true;

        if (movedPiece == 'R' &&
            move.fromRow == 7 &&
            move.fromCol == 0)
        {
            whiteLeftRookMoved = true;
        }

        if (movedPiece == 'R' &&
            move.fromRow == 7 &&
            move.fromCol == 7)
        {
            whiteRightRookMoved = true;
        }

        if (movedPiece == 'r' &&
            move.fromRow == 0 &&
            move.fromCol == 0)
        {
            blackLeftRookMoved = true;
        }

        if (movedPiece == 'r' &&
            move.fromRow == 0 &&
            move.fromCol == 7)
        {
            blackRightRookMoved = true;
        }

        //for pawn promotion
        char pieceToPlace = movedPiece;

if (move.promotion != '\0')
{
    // White promotion
    if (movedPiece == 'P')
    {
        pieceToPlace = move.promotion;
    }

    // Black promotion
    else if (movedPiece == 'p')
    {
        pieceToPlace =
            move.promotion - ('A' - 'a');
    }
}
        // Move piece on board
        board[move.toRow][move.toCol] =
    pieceToPlace;

        board[move.fromRow][move.fromCol] =
            '.';
        
        // Add moving piece at new square
        zobristHash ^=
    Zobrist::getPieceKey(pieceToPlace,move.toRow * 8 +move.toCol);

        lastMove = move;
        whiteTurn = !whiteTurn;
    }

    int newCastlingRights = getCastlingRights(*this);

    int newEpSquare = getEnPassantSquare();

    // Add new castling rights
    zobristHash ^= Zobrist::getCastlingKey(newCastlingRights);

    // Add new en-passant state
    if (newEpSquare != -1)
    {
        zobristHash ^= Zobrist::getEnPassantKey(newEpSquare);
    }

    // Add new side-to-move
    zobristHash ^= Zobrist::getSideKey();
    syncBitboards();
}
void Board::undoMove()
{
    if (history.empty())
        return;

    UndoInfo undo = history.back();
    history.pop_back();
    
    // Restore turn
    whiteTurn = undo.previousWhiteTurn;

    // Restore last move
    lastMove = undo.previouslastMove;

    // Restore castling rights
    whiteKingMoved = undo.whiteKingMoved;
    blackKingMoved = undo.blackKingMoved;

    whiteLeftRookMoved = undo.whiteLeftRookMoved;

    whiteRightRookMoved = undo.whiteRightRookMoved;

    blackLeftRookMoved = undo.blackLeftRookMoved;

    blackRightRookMoved = undo.blackRightRookMoved;
    whiteHasCastle = undo.previousWhiteHasCastled;
    blackHasCastle= undo.previousBlackHasCastled;

    if (undo.wasCastling)
{
    char movedPiece = undo.movedPiece;

    // White king-side
    if (movedPiece == 'K' &&
        undo.move.toCol == 6)
    {
        board[7][4] = 'K';
        board[7][6] = '.';

        board[7][7] = 'R';
        board[7][5] = '.';
    }

    // White queen-side
    else if (movedPiece == 'K' &&
             undo.move.toCol == 2)
    {
        board[7][4] = 'K';
        board[7][2] = '.';

        board[7][0] = 'R';
        board[7][3] = '.';
    }

    // Black king-side
    else if (movedPiece == 'k' &&
             undo.move.toCol == 6)
    {
        board[0][4] = 'k';
        board[0][6] = '.';

        board[0][7] = 'r';
        board[0][5] = '.';
    }

    // Black queen-side
    else if (movedPiece == 'k' &&
             undo.move.toCol == 2)
    {
        board[0][4] = 'k';
        board[0][2] = '.';

        board[0][0] = 'r';
        board[0][3] = '.';
    }
}
else if (undo.wasEnPassant)
{
    // Restore the moving pawn
    board[undo.move.fromRow][undo.move.fromCol] =
        undo.movedPiece;

    // Destination was empty before en passant
    board[undo.move.toRow][undo.move.toCol] =
        '.';

    // Restore the captured pawn
    board[undo.enPassantCapturedRow]
         [undo.enPassantCapturedCol] =
        undo.enPassantCapturedPiece;
}
else
{
    // Normal move
    board[undo.move.fromRow][undo.move.fromCol] =
        undo.movedPiece;

    board[undo.move.toRow][undo.move.toCol] =
        undo.capturedPiece;
}

zobristHash = undo.previousZobristHash;
syncBitboards();

}
void Board::makeNullMove()
{
    nullMoveHistory.push_back({ lastMove, zobristHash });

    int epSquare = getEnPassantSquare();
    if (epSquare != -1)
        zobristHash ^= Zobrist::getEnPassantKey(epSquare);

    lastMove = Move();               // clears en-passant eligibility
    zobristHash ^= Zobrist::getSideKey();
    whiteTurn = !whiteTurn;
}

void Board::undoNullMove()
{
    whiteTurn = !whiteTurn;
    lastMove = nullMoveHistory.back().first;
    zobristHash = nullMoveHistory.back().second;
    nullMoveHistory.pop_back();
}

bool Board::isValidCastle(const Move& move)
{
    char piece = board[move.fromRow][move.fromCol];

    
    // WHITE CASTLING
    if (piece == 'K')
    {
        // White kingside: e1 to g1
        if (move.fromRow == 7 &&
            move.fromCol == 4 &&
            move.toRow == 7 &&
            move.toCol == 6)
        {
            // King must not have moved
            if (whiteKingMoved)
                return false;

            // h1 rook must not have moved
            if (whiteRightRookMoved)
                return false;

            if (board[7][7] != 'R')
                return false;

            // f1 and g1 must be empty
            if (board[7][5] != '.' ||
                board[7][6] != '.')
                return false;

            // King cannot be in check,
            // pass through check, or land in check
            if (isSquareAttacked(7, 4, false) ||
                isSquareAttacked(7, 5, false) ||
                isSquareAttacked(7, 6, false))
                return false;

            return true;
        }

        // White queenside: e1 to c1
        if (move.fromRow == 7 &&
            move.fromCol == 4 &&
            move.toRow == 7 &&
            move.toCol == 2)
        {
            if (whiteKingMoved)
                return false;

            // a1 rook must not have moved
            if (whiteLeftRookMoved)
                return false;

            if (board[7][0] != 'R')
                return false;

            // b1, c1 and d1 must be empty
            if (board[7][1] != '.' ||
                board[7][2] != '.' ||
                board[7][3] != '.')
                return false;

            // e1, d1 and c1 cannot be attacked
            if (isSquareAttacked(7, 4, false) ||
                isSquareAttacked(7, 3, false) ||
                isSquareAttacked(7, 2, false))
                return false;

            return true;
        }
    }

    // BLACK CASTLING
    if (piece == 'k')
    {
        // Black kingside: e8 to g8
        if (move.fromRow == 0 &&
            move.fromCol == 4 &&
            move.toRow == 0 &&
            move.toCol == 6)
        {
            if (blackKingMoved)
                return false;

            // h8 rook must not have moved
            if (blackRightRookMoved)
                return false;

            if (board[0][7] != 'r')
                return false;
            // f8 and g8 must be empty
            if (board[0][5] != '.' ||
                board[0][6] != '.')
                return false;

            // e8, f8 and g8 cannot be attacked by White
            if (isSquareAttacked(0, 4, true) ||
                isSquareAttacked(0, 5, true) ||
                isSquareAttacked(0, 6, true))
                return false;

            return true;
        }

        // Black queenside: e8 to c8
        if (move.fromRow == 0 &&
            move.fromCol == 4 &&
            move.toRow == 0 &&
            move.toCol == 2)
        {
            if (blackKingMoved)
                return false;

            // a8 rook must not have moved
            if (blackLeftRookMoved)
                return false;

            if (board[0][0] != 'r')
                return false;

            // b8, c8 and d8 must be empty
            if (board[0][1] != '.' ||
                board[0][2] != '.' ||
                board[0][3] != '.')
                return false;

            // e8, d8 and c8 cannot be attacked by White
            if (isSquareAttacked(0, 4, true) ||
                isSquareAttacked(0, 3, true) ||
                isSquareAttacked(0, 2, true))
                return false;

            return true;
        }
    }

    return false;
}

bool Board::hasLegalMove(bool white)
{
    // Remember whose turn it actually was
    bool oldTurn = whiteTurn;

    // Temporarily test moves for the requested side
    whiteTurn = white;

    // Search every square
    for (int fromRow = 0; fromRow < 8; fromRow++)
    {
        for (int fromCol = 0; fromCol < 8; fromCol++)
        {
            char piece = board[fromRow][fromCol];

            // Skip empty squares
            if (piece == '.')
                continue;

            // Skip opponent's pieces
            if (white && !isWhitePiece(piece))
                continue;

            if (!white && !isBlackPiece(piece))
                continue;

            // Try every destination square
            for (int toRow = 0; toRow < 8; toRow++)
            {
                for (int toCol = 0; toCol < 8; toCol++)
                {
                    Move move(fromRow, fromCol, toRow, toCol);

                    if (isValidMove(move)){
                         whiteTurn = oldTurn;
                         return true;
                    }
                }
            }
        }
    }
    whiteTurn = oldTurn;
    return false;
}

bool Board::isCheckmate(bool white)
{
    if (!isKingInCheck(white))
        return false;

    return !hasLegalMove(white);
}

bool Board::isStalemate(bool white)
{
    if (isKingInCheck(white))
        return false;

    return !hasLegalMove(white);
}
char Board::getPiece(int row, int col) const
{
    return board[row][col];
}
void Board::setPiece(int row, int col, char piece)
{
    board[row][col] = piece;
     syncBitboards();
}

bool Board::canWhiteKingSideCastle() const
{
    return !whiteKingMoved &&
           !whiteRightRookMoved;
}

bool Board::canWhiteQueenSideCastle() const
{
    return !whiteKingMoved &&
           !whiteLeftRookMoved;
}

bool Board::canBlackKingSideCastle() const
{
    return !blackKingMoved &&
           !blackRightRookMoved;
}

bool Board::canBlackQueenSideCastle() const
{
    return !blackKingMoved &&
           !blackLeftRookMoved;
}

int Board::getEnPassantSquare() const
{
    if (lastMove.fromRow == -1)
        return -1;

    char pawn =
        board[lastMove.toRow][lastMove.toCol];

    if (pawn != 'P' && pawn != 'p')
        return -1;

    if (abs(lastMove.toRow - lastMove.fromRow) != 2)
        return -1;

    int targetRow =
        (lastMove.fromRow + lastMove.toRow) / 2;

    int targetCol =
        lastMove.fromCol;

    return targetRow * 8 + targetCol;
}

uint64_t Board::getZobristHash() const
{
    return zobristHash;
}