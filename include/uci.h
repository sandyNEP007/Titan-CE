#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "move.h"

#include <string>

class UCI
{
private:
    static Board board;

    // Convert internal Move to UCI notation
    static void printMove(const Move& move);

    // Apply one UCI move to the current board
    static bool applyMove(const std::string& moveString);

    // Handle: position startpos [moves ...]
    static void handlePosition(const std::string& command);

    // Handle: go depth N
    static void handleGo(const std::string& command);

public:
    static void loop();
};

#endif