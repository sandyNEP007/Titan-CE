#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "move.h"
#include <string>

class Game
{
private:
    Board board;

public:
    Game();

    void start();
    Move getPlayerMove();
};

#endif
