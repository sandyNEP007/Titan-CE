#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <chrono>


class EngineSearch
{
public:
    static Move findBestMove(Board& board, int maxdepth, long long timeLimitMs = 0);
    static int getMoveOrderingScore(Board& board, const Move& move, int depth);
    static int getMVVLVAScore(Board& board, const Move& move);
    static int see(Board& board, const Move& move);
    static int seeValue(char piece);
    static void clearTranspositionTable();

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
    static std::chrono::steady_clock::time_point searchDeadline;
    static bool stopSearch;
    static bool checkTimeUp();
     static bool hasTimeForNextDepth(std::chrono::steady_clock::time_point searchStart,long long timeLimitMs,long long lastDepthMs);
    static const int LMR_MIN_DEPTH = 3;
    static const int LMR_MIN_MOVE_INDEX = 4;
    static const int CHECK_EXTENSION = 1;
    static const int FUTILITY_MAX_DEPTH = 2;        // only applies at depth <= this
    static const int FUTILITY_MARGIN_PER_PLY = 150;
    static const size_t TT_SIZE = 1 << 20;   // 1,048,576 entries (power of two)
    static TTEntry transpositionTable[TT_SIZE];
    static int negamax(Board& board, int alpha, int beta, int depth, int ply);
    static int quiescence(Board& board, int alpha, int beta, int checkPly);
    static const int MAX_PLY = 64;

    // Mate scoring
    static const int MATE_VALUE = 100000;
    static const int MATE_THRESHOLD = MATE_VALUE - MAX_PLY;
    static int scoreToTT(int score, int ply);
    static int scoreFromTT(int score, int ply);

    // Quiescence check-extension
    static const int MAX_QUIESCENCE_CHECK_PLY = 1;
    static bool givesCheck(Board& board, const Move& move);
    static bool hasNonPawnMaterial(Board& board);
    static Move killerMoves[MAX_PLY][2];
    static bool isCapture(Board& board, const Move& move);
    
    static int historyTable[2][64][64];
    
    
};

#endif