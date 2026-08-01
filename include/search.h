#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "move.h"

class Search
{
public:


    Search(Board& board);

    
    Move findBestMove(int depth,
                      bool maximizingPlayer);

   
    int minimax(int depth,
                int alpha,
                int beta,
                bool maximizingPlayer);

private:

  
    Board& board;

};

#endif