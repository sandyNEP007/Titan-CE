#ifndef ITERATIVEDEEPENING_H
#define ITERATIVEDEEPENING_H

#include "board.h"

class IterativeDeepening
{
public:

    Move search(Board& board,
                bool maximizingPlayer,
                int maxDepth);

};

#endif