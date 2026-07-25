#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include <unordered_map>
#include <cstdint>
#include "move.h"
struct TTEntry
{
    uint64_t hash;

    int evaluation;

    int depth;

    Move bestMove;
};

#endif