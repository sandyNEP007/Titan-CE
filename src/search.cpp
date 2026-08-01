#include "search.h"

#include <algorithm>
#include <limits>

using namespace std;


Search::Search(Board& board)
    : board(board)
{
}


Move Search::findBestMove(int depth,
                          bool maximizingPlayer)
{
    return Move();
}



int Search::minimax(int depth,
                    int alpha,
                    int beta,
                    bool maximizingPlayer)
{
    return 0;
}