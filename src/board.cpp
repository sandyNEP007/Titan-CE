#include <random>
#include <iostream>
#include <cstdlib>
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

    // Save the current state for undo
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

history.push_back(undo);

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
                move.fromRow == 7 &&
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
bool Board::isKingInCheck(bool whiteKing){
    int kingRow = -1;
    int kingCol = -1;

    // Find the king
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            if ((whiteKing && board[row][col] == 'K') ||
                (!whiteKing && board[row][col] == 'k'))
            {
                kingRow = row;
                kingCol = col;
            }
        }
    }

    // Check every enemy piece
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board[row][col];

            // Is this an enemy piece?
            if ((whiteKing && isBlackPiece(piece)) ||
                (!whiteKing && isWhitePiece(piece)))
            {
                Move move(row, col, kingRow, kingCol);

                if (isValidMove(move))
                {
                    return true;
                }
            }
        }
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
    board[move.toRow][move.toCol] = sourcePiece;
    board[move.fromRow][move.fromCol] = '.';

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
    board[move.fromRow][move.fromCol] = sourcePiece;
    board[move.toRow][move.toCol] = destinationPiece;

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


bool Board::isCheckmate(bool whiteKing)
{
    if (!isKingInCheck(whiteKing))
        return false;

    if (hasLegalMove(whiteKing))
        return false;

    return true;
}
bool Board::isStalemate(bool whitePlayer){
    
    if (isKingInCheck(whitePlayer))
        return false;

    if (hasLegalMove(whitePlayer))
        return false;

    return true;
}
void Board::pawnPromotion()
{
    // White promotion
    for (int col = 0; col < 8; col++)
    {
        if (board[0][col] == 'P')
        {
            board[0][col] = 'Q';
        }
    }

    // Black promotion
    for (int col = 0; col < 8; col++)
    {
        if (board[7][col] == 'p')
        {
            board[7][col] = 'q';
        }
    }
}
int Board::BoardEvaluation()
{
    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            switch (board[row][col])
            {
                case 'P': 
                score += 100; 
                score+=PAWN_PST[row][col];
                break;
                //------------
                case 'N': 
                score += 320; 
                score+=KNIGHT_PST[row][col];
                break;
                //-------------
                case 'B': score += 330; break;
                case 'R': score += 500; break;
                case 'Q': score += 900; break;
                //------------
                case 'p': 
                score -= 100; 
                score-=PAWN_PST[7-row][col];
                break;
                //------------
                case 'n': 
                score -= 320; 
                score-=KNIGHT_PST[7-row][col];
                break;
                //-------------
                case 'b': score -= 330; break;
                case 'r': score -= 500; break;
                case 'q': score -= 900; break;
            }
        }
    }

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


int Board::minimax(int depth, int alpha, int beta, bool maximizingPlayer){
    uint64_t hash = generateHash();
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end() &&
    it->second.depth >= depth)
{
    
    return it->second.evaluation;
}
    if (depth == 0)
    {
        return BoardEvaluation();
    }

    std::vector<Move> legalMoves = generateLegalMoves(maximizingPlayer);
    if (legalMoves.empty())
{
    return BoardEvaluation();
}

if (maximizingPlayer)
{
int bestScore = -1000000;

for (Move move : legalMoves)
{
    makeMove(move);

    int score = minimax(depth - 1, alpha, beta, !maximizingPlayer);

    undoMove(move);

    if (score > bestScore)
    {
        bestScore = score;
        
    }
    alpha = std::max(alpha, bestScore);
    if (alpha >= beta)
{
    break;
}
}
TTEntry entry;

entry.hash = hash;
entry.evaluation = bestScore;
entry.depth = depth;
entry.bestMove = Move();

transpositionTable[hash] = entry;


return bestScore;
}
else
{
int bestScore = 1000000;

for (Move move : legalMoves)
{
    makeMove(move);

    int score = minimax(depth - 1, alpha, beta, !maximizingPlayer);

    undoMove(move);

    if (score < bestScore)
    {
        bestScore = score;
        
    }
    beta = std::min(beta, bestScore);

if (alpha >= beta)
{
    break;
}
}
TTEntry entry;

entry.hash = hash;
entry.evaluation = bestScore;
entry.depth = depth;
entry.bestMove = Move();

transpositionTable[hash] = entry;

     
return bestScore;
}

}


void Board::undoMove(const Move& move)
{
   UndoInfo undo = history.back();
    history.pop_back();

    board[undo.move.fromRow][undo.move.fromCol] = undo.movedPiece;
    board[undo.move.toRow][undo.move.toCol] = undo.capturedPiece;

    whiteKingMoved = undo.whiteKingMoved;
    blackKingMoved = undo.blackKingMoved;

    whiteLeftRookMoved = undo.whiteLeftRookMoved;
    whiteRightRookMoved = undo.whiteRightRookMoved;

    blackLeftRookMoved = undo.blackLeftRookMoved;
    blackRightRookMoved = undo.blackRightRookMoved;
}

Move Board::findBestMove(int depth, bool whitePlayer)
{
    std::vector<Move> legalMoves = generateLegalMoves(whitePlayer);

    Move bestMove = legalMoves[0];
     int alpha=-1000000;
     int beta=1000000;

    int bestScore;

    if (whitePlayer)
    {
        bestScore = -1000000;
    }
    else
    {
        bestScore = 1000000;
    }
    std::vector<RootMove> rootMoves;
    for (Move move : legalMoves)
    {
        makeMove(move);
        
        int score = minimax(depth - 1, alpha, beta, !whitePlayer);
        rootMoves.push_back({move, score});

        undoMove(move);
       

        if (whitePlayer)
        {
            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
            
        }
        else
        {
            if (score < bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
        }
    }
     cout << "\n========== ROOT ANALYSIS ==========\n";

for (const RootMove& rm : rootMoves)
{
    cout
        << char('a' + rm.move.fromCol)
        << 8 - rm.move.fromRow
        << " -> "
        << char('a' + rm.move.toCol)
        << 8 - rm.move.toRow
        << "    Score: "
        << rm.score
        << endl;
}
cout << "\nBest Move : "
     << char('a' + bestMove.fromCol)
     << 8 - bestMove.fromRow
     << " -> "
     << char('a' + bestMove.toCol)
     << 8 - bestMove.toRow
     << endl;

cout << "Best Score: " << bestScore << endl;
        
    return bestMove;
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
    cout << "------------------------------------------\n";

    cout << "Depth  : " << depth << endl;

    cout << "Move   : "
         << char('a' + move.fromCol)
         << 8 - move.fromRow
         << " -> "
         << char('a' + move.toCol)
         << 8 - move.toRow
         << endl;

    cout << "Score  : " << score << endl;

    cout << "Action : " << action << endl;

    cout << "------------------------------------------\n";
}