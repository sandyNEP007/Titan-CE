#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "move.h"

#include <string>

class UCI
{
private:
    static Board board;

    static void printMove(const Move& move);

    static bool applyMove(const std::string& moveString);

    static bool loadFEN(const std::string& fen);

    static void handlePosition(const std::string& command);

    static void handleGo(const std::string& command);

    static void handleSetOption(const std::string& command);

public:
    static void loop();
    static void show();
};

#endif