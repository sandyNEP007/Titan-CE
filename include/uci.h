#ifndef UCI_H
#define UCI_H

#include <string>
#include "board.h"

class UCI
{
private:
    Board board;
    bool whiteToMove = true;

public:
    void loop();

private:
    void handleCommand(const std::string& command);

    void position(const std::string& command);

    Move parseMove(const std::string& moveString);

    std::string moveToString(const Move& move);

    void go();
};

#endif