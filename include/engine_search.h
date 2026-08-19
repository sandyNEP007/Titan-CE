#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

#include <cstdint>
#include <unordered_map>
#include <vector>


class EngineSearch
{
public:
    static Move findBestMove(Board& board, int maxdepth);
    static int getMoveOrderingScore(Board& board, const Move& move, int depth);
    static int getMVVLVAScore(Board& board, const Move& move);

    static long long nodes;
    static long long cutoffs;

    static long long qNodes;
    static long long qCutoffs;
    static long long ttHits;

private:
enum TTFlag
{
    TT_EXACT,
    TT_ALPHA,
    TT_BETA
};

struct TTEntry
{
    uint64_t hash;
    int depth;
    int score;
    TTFlag flag;
    Move bestMove;
};
    static std::unordered_map<uint64_t, TTEntry> transpositionTable;
    static int negamax(Board& board, int depth, int alpha, int beta);
    static int quiescence(Board& board, int alpha, int beta);
    static const int MAX_PLY = 64;
    static Move killerMoves[MAX_PLY][2];
    static bool isCapture(Board& board, const Move& move);
    static int historyTable[2][64][64];
    
    
};

#endif