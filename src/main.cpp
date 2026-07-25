#include "board.h"
#include <iostream>

using namespace std;

int main()
{
    Board board;

    


cout << board.BoardEvaluation() << endl;

board.findBestMove(3, true);
    

    return 0;
}