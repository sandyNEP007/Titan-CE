#include "game.h"
#include<iostream>
using namespace std;


Move Game::getPlayerMove()
{
    string from;
    string to;

    cout << "Enter move (example: e2 e4): ";
    cin >> from >> to;

    int fromCol = from[0] - 'a';
    int fromRow = 8 - (from[1] - '0');

    int toCol = to[0] - 'a';
    int toRow = 8 - (to[1] - '0');

    Move move(fromRow, fromCol, toRow, toCol);

    return move;
}Game::Game()
{
    board.initialize();
    board.setupPieces();
}
void Game::start(){
      while (true)
    {
        board.display();

        Move move = getPlayerMove();

        board.makeMove(move);
    }
}