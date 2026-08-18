#include "zobrist.h"
#include "board.h"
#include <random>

uint64_t Zobrist::pieceKeys[12][64];
uint64_t Zobrist::sideKey;
uint64_t Zobrist::castlingKeys[16];
uint64_t Zobrist::enPassantKeys[64];

uint64_t Zobrist::random64()
{
    static std::mt19937_64 rng(std::random_device{}());

    return rng();
}

void Zobrist::initialize()
{
    for (int piece = 0; piece < 12; piece++)
    {
        for (int square = 0; square < 64; square++)
        {
            pieceKeys[piece][square] =
                random64();
        }
    }

    sideKey = random64();

    for (int i = 0; i < 16; i++)
    {
        castlingKeys[i] = random64();
    }

    for (int i = 0; i < 64; i++)
    {
        enPassantKeys[i] = random64();
    }
}

int Zobrist::pieceIndex(char piece)
{
    switch (piece)
    {
        case 'P': return 0;
        case 'N': return 1;
        case 'B': return 2;
        case 'R': return 3;
        case 'Q': return 4;
        case 'K': return 5;

        case 'p': return 6;
        case 'n': return 7;
        case 'b': return 8;
        case 'r': return 9;
        case 'q': return 10;
        case 'k': return 11;
    }

    return -1;
}

uint64_t Zobrist::generateHash(const Board& board)
{
    uint64_t hash = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board.getPiece(row, col);

            if (piece == '.')
                continue;

            int pieceID = pieceIndex(piece);

            int square = row * 8 + col;

            hash ^= pieceKeys[pieceID][square];
        }
    }

    // Side to move
    if (!board.isWhiteTurn())
    {
        hash ^= sideKey;
    }
    int castlingRights = 0;

if (board.canWhiteKingSideCastle())
    castlingRights |= 1;

if (board.canWhiteQueenSideCastle())
    castlingRights |= 2;

if (board.canBlackKingSideCastle())
    castlingRights |= 4;

if (board.canBlackQueenSideCastle())
    castlingRights |= 8;

hash ^= castlingKeys[castlingRights];

int epSquare = board.getEnPassantSquare();

if (epSquare != -1)
{
    hash ^= enPassantKeys[epSquare];
}


    return hash;
}

uint64_t Zobrist::getPieceKey(char piece, int square)
{
    int index = pieceIndex(piece);

    if (index == -1)
        return 0;

    return pieceKeys[index][square];
}

uint64_t Zobrist::getSideKey()
{
    return sideKey;
}

uint64_t Zobrist::getCastlingKey(int rights)
{
    return castlingKeys[rights];
}

uint64_t Zobrist::getEnPassantKey(int square)
{
    if (square < 0 || square >= 64)
        return 0;

    return enPassantKeys[square];
}

