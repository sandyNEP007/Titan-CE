#include "uci.h"
#include "movegen.h"
#include "engine_search.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Board UCI::board;


// Convert Titan's Move to UCI coordinate notation
void UCI::printMove(const Move& move)
{
    char fromFile = 'a' + move.fromCol;
    char fromRank = '8' - move.fromRow;

    char toFile = 'a' + move.toCol;
    char toRank = '8' - move.toRow;

    std::cout << fromFile << fromRank
              << toFile << toRank;

    if (move.promotion != '\0')
    {
        std::cout << move.promotion;
    }
}


// Apply a UCI move such as:
// e2e4
// e7e8q
bool UCI::applyMove(const std::string& moveString)
{
    if (moveString.length() < 4)
        return false;

    Move requested;

    try
    {
        requested = Move::parseMove(moveString);
    }
    catch (...)
    {
        return false;
    }

    // Generate legal moves for whichever side
    // Board says is currently to move.
    std::vector<Move> legalMoves =
        MoveGenerator::generateLegalMoves(board);

    for (const Move& move : legalMoves)
    {
        if (move.fromRow != requested.fromRow)
            continue;

        if (move.fromCol != requested.fromCol)
            continue;

        if (move.toRow != requested.toRow)
            continue;

        if (move.toCol != requested.toCol)
            continue;

        // Promotion must match exactly.
        if (move.promotion != requested.promotion)
            continue;

        // Board::makeMove() is responsible for
        // changing whiteTurn.
        board.makeMove(move);

        return true;
    }

    return false;
}


// Handle:
//
// position startpos
//
// position startpos moves e2e4
//
// position startpos moves e2e4 e7e5 g1f3
void UCI::handlePosition(const std::string& command)
{
    std::stringstream ss(command);

    std::string token;

    // Skip "position"
    ss >> token;

    // Get position type
    ss >> token;

    if (token == "startpos")
    {
        // Board constructor creates the initial position.
        // This also resets whiteTurn.
        board = Board();
    }
    else
    {
        // FEN support not implemented yet.
        std::cerr << "info string Unsupported position type: "
                  << token << "\n";
        return;
    }

    // Check whether "moves" exists.
    if (!(ss >> token))
        return;

    if (token != "moves")
        return;

    // Apply every move sequentially.
    std::string moveString;

    while (ss >> moveString)
    {
        if (!applyMove(moveString))
        {
            std::cerr
                << "info string Illegal move: "
                << moveString
                << "\n";

            return;
        }
    }
}


// Handle:
//
// go
//
// go depth 6
void UCI::handleGo(const std::string& command)
{
    std::stringstream ss(command);

    std::string token;

    // Skip "go"
    ss >> token;

    int depth = 6;

    while (ss >> token)
    {
        if (token == "depth")
        {
            if (ss >> depth)
            {
                if (depth < 1)
                    depth = 1;
            }
        }
    }

    // Ask the search to find a move for the
    // side currently stored in Board.
    Move bestMove =
        EngineSearch::findBestMove(board, depth);

    std::cout << "bestmove ";

    printMove(bestMove);

    std::cout << "\n";
}


// Main UCI command loop
void UCI::loop()
{
    std::string command;

    while (std::getline(std::cin, command))
    {
        // Ignore empty lines.
        if (command.empty())
            continue;


        // -------------------------
        // UCI identification
        // -------------------------
        if (command == "uci")
        {
            std::cout << "id name Titan-CE\n";
            std::cout << "id author Sandeep Poudel\n";
            std::cout << "uciok\n";
        }


        // -------------------------
        // Ready check
        // -------------------------
        else if (command == "isready")
        {
            std::cout << "readyok\n";
        }


        // -------------------------
        // New game
        // -------------------------
        else if (command == "ucinewgame")
        {
            board = Board();
        }


        // -------------------------
        // Position
        // -------------------------
        else if (command.rfind("position", 0) == 0)
        {
            handlePosition(command);
        }


        // -------------------------
        // Search
        // -------------------------
        else if (command.rfind("go", 0) == 0)
        {
            handleGo(command);
        }


        // -------------------------
        // Stop
        // -------------------------
        else if (command == "stop")
        {
            // Stop support will be added later.
        }


        // -------------------------
        // Quit
        // -------------------------
        else if (command == "quit")
        {
            break;
        }
    }
}