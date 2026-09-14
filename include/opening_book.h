#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include "move.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>

class OpeningBook
{
public:
  
    static void load(const std::string& path);
    static bool getBookMove(uint64_t positionHash, Move& outMove);
    static bool isLoaded();

private:
    struct WeightedMove
    {
        Move move;
        int weight;
    };

    static std::unordered_map<uint64_t, std::vector<WeightedMove>> book;
    static bool loaded;
};

#endif