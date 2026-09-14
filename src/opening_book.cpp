#include "opening_book.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

std::unordered_map<uint64_t, std::vector<OpeningBook::WeightedMove>> OpeningBook::book;
bool OpeningBook::loaded = false;

// File format (plain text, one entry per line, whitespace-separated):
//   hash fromRow fromCol toRow toCol promotion weight
// promotion is a single char, '-' meaning none (not '\0', to keep the
// file human-readable/editable). One position can have multiple lines
// (multiple candidate replies) — this is exactly how transpositions from
// different move orders end up sharing one hash with several options.
void OpeningBook::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return;

    book.clear();

    uint64_t hash;
    int fromRow, fromCol, toRow, toCol;
    char promotion;
    int weight;

    while (file >> std::hex >> hash >> std::dec
                >> fromRow >> fromCol >> toRow >> toCol
                >> promotion >> weight)
    {
        char promoChar = (promotion == '-') ? '\0' : promotion;

        Move m(fromRow, fromCol, toRow, toCol, promoChar);
        book[hash].push_back({ m, weight });
    }

    loaded = true;
}

bool OpeningBook::isLoaded()
{
    return loaded;
}

bool OpeningBook::getBookMove(uint64_t positionHash, Move& outMove)
{
    if (!loaded)
        return false;

    auto it = book.find(positionHash);
    if (it == book.end() || it->second.empty())
        return false;

    const std::vector<WeightedMove>& candidates = it->second;

    int totalWeight = 0;
    for (const auto& c : candidates)
        totalWeight += c.weight;

    if (totalWeight <= 0)
        return false;

    static bool seeded = false;
    if (!seeded) { std::srand((unsigned)std::time(nullptr)); seeded = true; }

    int pick = std::rand() % totalWeight;

    for (const auto& c : candidates)
    {
        pick -= c.weight;
        if (pick < 0)
        {
            outMove = c.move;
            return true;
        }
    }

    outMove = candidates.back().move;   
    return true;
}