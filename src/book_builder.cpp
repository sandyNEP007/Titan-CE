#include "board.h"
#include "zobrist.h"
#include "move.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

struct BookKey
{
    uint64_t hash;
    int fromRow, fromCol, toRow, toCol;
    char promotion;

    bool operator<(const BookKey& other) const
    {
        if (hash != other.hash) return hash < other.hash;
        if (fromRow != other.fromRow) return fromRow < other.fromRow;
        if (fromCol != other.fromCol) return fromCol < other.fromCol;
        if (toRow != other.toRow) return toRow < other.toRow;
        if (toCol != other.toCol) return toCol < other.toCol;
        return promotion < other.promotion;
    }
};

int buildBook(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: book_builder <games.txt> <output_book.txt> [maxPlies]\n";
        std::cerr << "  games.txt: one game per line, space-separated UCI moves\n";
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    int maxPlies = (argc >= 4) ? std::atoi(argv[3]) : 12;

    Zobrist::initialize();

    std::ifstream in(inputPath);
    if (!in.is_open())
    {
        std::cerr << "Could not open " << inputPath << "\n";
        return 1;
    }

    std::map<BookKey, int> counts;

    std::string line;
    long long gameCount = 0;

    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string uciMove;

        Board board;
        int ply = 0;

        while (iss >> uciMove && ply < maxPlies)
        {
            Move move = Move::parseMove(uciMove);

            if (move.fromRow == -1)
                break;   // malformed move string — stop this game

            uint64_t hashBefore = board.getZobristHash();

            BookKey key{
                hashBefore,
                move.fromRow, move.fromCol,
                move.toRow, move.toCol,
                move.promotion == '\0' ? '-' : move.promotion
            };
            counts[key]++;

            board.makeMove(move);
            ply++;
        }

        gameCount++;
        if (gameCount % 10000 == 0)
            std::cerr << "Processed " << gameCount << " games...\n";
    }

    std::ofstream out(outputPath);
    for (const auto& [key, weight] : counts)
    {
        out << std::hex << key.hash << std::dec << " "
            << key.fromRow << " " << key.fromCol << " "
            << key.toRow << " " << key.toCol << " "
            << key.promotion << " "
            << weight << "\n";
    }

    std::cerr << "Done. " << gameCount << " games processed, "
              << counts.size() << " book entries written to "
              << outputPath << "\n";

    return 0;
}