#include "uci.h"

#include <iostream>
#include <sstream>

using namespace std;

void UCI::loop()
{
    string command;

    while (getline(cin, command))
    {
        handleCommand(command);
    }
}

void UCI::handleCommand(const string& command)
{
    if (command == "uci")
    {
        cout << "id name Titan" << endl;
        cout << "id author Sandeep Poudel" << endl;
        cout << "uciok" << endl;
    }

    else if (command == "isready")
    {
        cout << "readyok" << endl;
    }
    else if (command == "ucinewgame")
{
    board = Board();
}

else if (command.rfind("position", 0) == 0)
{
    position(command);
}

else if (command == "go")
{
    go();
}
else if(command == "quit")
{
    exit(0);
}
}
Move UCI::parseMove(const std::string& moveString)
{
    int fromCol = moveString[0] - 'a';
    int fromRow = 8 - (moveString[1] - '0');

    int toCol = moveString[2] - 'a';
    int toRow = 8 - (moveString[3] - '0');

    return Move(fromRow, fromCol, toRow, toCol);
}

std::string UCI::moveToString(const Move& move)
{
    std::string s;

    s += char('a' + move.fromCol);
    s += char('8' - move.fromRow);

    s += char('a' + move.toCol);
    s += char('8' - move.toRow);

    return s;
}

void UCI::position(const std::string& command)
{
    board = Board();
    whiteToMove = true;

    std::stringstream ss(command);

    std::string token;

    ss >> token;          // position
    ss >> token;          // startpos

    if (!(ss >> token))
        return;

    if (token != "moves")
        return;

    while (ss >> token)
    {
        Move move = parseMove(token);
        board.makeMove(move);
        whiteToMove = !whiteToMove;
    }
}
void UCI::go()
{
    Move bestMove = board.findBestMove(3, whiteToMove);

    std::cout << "bestmove "
              << moveToString(bestMove)
              << std::endl;

    board.makeMove(bestMove);

    whiteToMove = !whiteToMove;
}