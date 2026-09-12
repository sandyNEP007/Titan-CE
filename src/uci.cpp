#include "uci.h"
#include "movegen.h"
#include "engine_search.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include "move.h"

Board UCI::board;


void UCI::show()
{
    std::cout << "id name Titan-CE\n";
    std::cout << "id author Sandeep Poudel\n";
}

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
        std::cout << static_cast<char>(
            std::tolower(static_cast<unsigned char>(move.promotion))
        );
    }
}

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

        // Promotion handling
        if (requested.promotion != '\0' &&
            move.promotion != requested.promotion)
        {
            continue;
        }

        board.makeMove(move);

        return true;
    }

    return false;
}


// ============================================================
// LOAD FEN
// ============================================================

bool UCI::loadFEN(const std::string& fen)
{
    std::stringstream ss(fen);

    std::string boardPart;
    std::string sidePart;
    std::string castlingPart;
    std::string enPassantPart;
    std::string halfmovePart;
    std::string fullmovePart;

    if (!(ss >> boardPart
             >> sidePart
             >> castlingPart
             >> enPassantPart
             >> halfmovePart
             >> fullmovePart))
    {
        return false;
    }

    // Start with a fresh board.
    board = Board();

    int row = 0;
    int col = 0;


    for (char c : boardPart)
    {
        if (c == '/')
        {
            if (col != 8)
                return false;

            row++;
            col = 0;

            continue;
        }

        if (row >= 8)
            return false;

        if (c >= '1' && c <= '8')
        {
            col += c - '0';
        }
        else
        {
            if (col >= 8)
                return false;

            board.setPiece(row, col, c);
            col++;
        }

        if (col > 8)
            return false;
    }

    if (row != 7 || col != 8)
        return false;

    if (sidePart == "w")
    {
        board.whiteTurn = true;
    }
    else if (sidePart == "b")
    {
        board.whiteTurn = false;
    }
    else
    {
        return false;
    }

    return true;
}

void UCI::handlePosition(const std::string& command)
{
    std::stringstream ss(command);

    std::string token;

    // Skip "position"
    ss >> token;

    // Read "startpos" or "fen"
    ss >> token;

    if (token == "startpos")
    {
        board = Board();

        // Check if moves exist
        if (!(ss >> token))
            return;

        if (token != "moves")
            return;

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

        return;
    }

    if (token == "fen")
    {
        std::string boardPart;
        std::string sidePart;
        std::string castlingPart;
        std::string enPassantPart;
        std::string halfmovePart;
        std::string fullmovePart;

        /*
            FEN always has six fields.
        */

        if (!(ss >> boardPart
                 >> sidePart
                 >> castlingPart
                 >> enPassantPart
                 >> halfmovePart
                 >> fullmovePart))
        {
            std::cerr
                << "info string Invalid FEN\n";

            return;
        }

        std::string fen =
            boardPart + " " +
            sidePart + " " +
            castlingPart + " " +
            enPassantPart + " " +
            halfmovePart + " " +
            fullmovePart;

        if (!loadFEN(fen))
        {
            std::cerr
                << "info string Invalid FEN\n";

            return;
        }

        if (!(ss >> token))
            return;

        if (token != "moves")
            return;

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

        return;
    }


    // Unknown position type
    std::cerr
        << "info string Unsupported position type\n";
}

void UCI::handleGo(const std::string& command)
{
    std::stringstream ss(command);
    std::string token;
    ss >> token; // "go"

    int depth = 7;
    long long movetime = 0;
    long long wtime = 0, btime = 0, winc = 0, binc = 0;
    bool hasMovetime = false;
    bool hasClock = false;

    while (ss >> token)
    {
        if (token == "depth")        { ss >> depth; if (depth < 1) depth = 1; }
        else if (token == "movetime"){ ss >> movetime; hasMovetime = true; }
        else if (token == "wtime")   { ss >> wtime; hasClock = true; }
        else if (token == "btime")   { ss >> btime; hasClock = true; }
        else if (token == "winc")    { ss >> winc; }
        else if (token == "binc")    { ss >> binc; }
    }

       long long timeLimitMs = 0;

if (hasMovetime)
{
    timeLimitMs = movetime - 50;
}
else if (hasClock)
{
    long long myTime = board.isWhiteTurn() ? wtime : btime;
    long long myInc  = board.isWhiteTurn() ? winc  : binc;

  
    //to spend more of remaining time reduce the devisor ie 20ms
    timeLimitMs = myTime / 20 + myInc - 50; 
}

else
{
   
    timeLimitMs = 30000; // e.g. 50 seconds default per move
}

// Lower bound safety check
if (timeLimitMs < 5000)
    timeLimitMs = 5000;

//max time allowed to make a move
if (timeLimitMs > 30000) 
    timeLimitMs = 30000;


    Move bestMove = EngineSearch::findBestMove(board, depth, timeLimitMs);
    std::cout<<"info depth "<<depth<<std::endl;
    std::cout << "bestmove ";
    printMove(bestMove);
    std::cout << "\n";
    std::cout.flush();
}

void UCI::handleSetOption(const std::string& command)
{
   

    (void)command;
}

void UCI::loop()
{
    std::string command;

    while (std::getline(std::cin, command))
    {
        if (command.empty())
            continue;

        if (command == "uci")
        {
            std::cout << "id name Titan-CE\n";
            std::cout << "id author Sandeep Poudel\n";
            std::cout << "option name UCI_LimitStrength type check default false\n";
            std::cout << "option name UCI_Elo type spin default 1400 min 100 max 4000\n";

            std::cout << "uciok\n";

            std::cout.flush();
        }

        else if (command == "isready")
        {
            std::cout << "readyok\n";

            std::cout.flush();
        }


        else if (
            command.rfind("setoption", 0) == 0)
        {
            handleSetOption(command);
        }

        else if (command == "ucinewgame")
        {
            EngineSearch::clearTranspositionTable();
            board = Board();
        }



        else if (
            command.rfind("position", 0) == 0)
        {
            handlePosition(command);
        }

        else if (
            command.rfind("go", 0) == 0)
        {
            handleGo(command);
        }

        else if (command == "stop")
        {
           
        }

        else if (command == "quit")
        {
            break;
        }
    }
}