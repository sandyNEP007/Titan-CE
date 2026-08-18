#pragma once
#include <cstdint>

class Board;
class Zobrist
{
public:
    static void initialize();
    static uint64_t getPieceKey(char piece, int square);
    static uint64_t getSideKey();
    static uint64_t getCastlingKey(int rights);
    static uint64_t getEnPassantKey(int square);
    static uint64_t generateHash(const Board& board);

private:
    static uint64_t pieceKeys[12][64];
    static uint64_t sideKey;
    static uint64_t castlingKeys[16];
    static uint64_t enPassantKeys[64];
    static uint64_t random64();

    static int pieceIndex(char piece);
};