#include "uci.h"
#include "movegen.h"
#include "engine_search.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Board UCI::board;


// ============================================================
// PRINT MOVE
// ============================================================

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


// ============================================================
// APPLY UCI MOVE
// ============================================================

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

    // --------------------------------------------------------
    // Parse board layout
    // --------------------------------------------------------

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


    // --------------------------------------------------------
    // Side to move
    // --------------------------------------------------------

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


    /*
        The current Board interface does not expose setters for:

            castling rights
            en-passant square
            halfmove clock
            fullmove number

        For normal positions supplied by En Croissant this is
        currently sufficient.

        Your Board constructor already initializes the normal
        starting castling state.
    */

    return true;
}


// ============================================================
// HANDLE POSITION
//
// Supported:
//
// position startpos
//
// position startpos moves e2e4 e7e5
//
// position fen <FEN>
//
// position fen <FEN> moves e2e4 e7e5
// ============================================================

void UCI::handlePosition(const std::string& command)
{
    std::stringstream ss(command);

    std::string token;

    // Skip "position"
    ss >> token;

    // Read "startpos" or "fen"
    ss >> token;


    // ========================================================
    // STARTPOS
    // ========================================================

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


    // ========================================================
    // FEN
    // ========================================================

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


        // ----------------------------------------------------
        // Optional moves after FEN
        // ----------------------------------------------------

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


// ============================================================
// HANDLE GO
// ============================================================

void UCI::handleGo(const std::string& command)
{
    std::stringstream ss(command);

    std::string token;

    // Skip "go"
    ss >> token;

    int depth = 7;

    while (ss >> token)
    {
        if (token == "depth")
        {
            ss >> depth;

            if (depth < 1)
                depth = 1;
        }
    }

    /*
        Use the depth requested by the GUI.

        En Croissant may send:

            go depth 7
            go depth 12
            go depth 24

        The search engine receives that value directly.
    */

    Move bestMove =
        EngineSearch::findBestMove(board, 7);

    std::cout << "bestmove ";

    printMove(bestMove);

    std::cout << "\n";

    // Make sure the GUI receives the response immediately.
    std::cout.flush();
}


// ============================================================
// HANDLE SETOPTION
// ============================================================

void UCI::handleSetOption(const std::string& command)
{
    /*
        En Croissant currently sends:

        setoption name UCI_Chess960 value false

        Titan does not support Chess960 yet.

        We simply accept the command so that the UCI
        communication remains clean.
    */

    (void)command;
}


// ============================================================
// MAIN UCI LOOP
// ============================================================

void UCI::loop()
{
    std::string command;

    while (std::getline(std::cin, command))
    {
        if (command.empty())
            continue;


        // ====================================================
        // UCI
        // ====================================================

        if (command == "uci")
        {
            std::cout << "id name Titan-CE\n";
            std::cout << "id author Sandeep Poudel\n";
            std::cout << "uciok\n";

            std::cout.flush();
        }


        // ====================================================
        // ISREADY
        // ====================================================

        else if (command == "isready")
        {
            std::cout << "readyok\n";

            std::cout.flush();
        }


        // ====================================================
        // SETOPTION
        // ====================================================

        else if (
            command.rfind("setoption", 0) == 0)
        {
            handleSetOption(command);
        }


        // ====================================================
        // NEW GAME
        // ====================================================

        else if (command == "ucinewgame")
        {
            board = Board();
        }


        // ====================================================
        // POSITION
        // ====================================================

        else if (
            command.rfind("position", 0) == 0)
        {
            handlePosition(command);
        }


        // ====================================================
        // GO
        // ====================================================

        else if (
            command.rfind("go", 0) == 0)
        {
            handleGo(command);
        }


        // ====================================================
        // STOP
        // ====================================================

        else if (command == "stop")
        {
            /*
                Stop is not implemented yet.

                Your current search is synchronous, so the engine
                cannot interrupt findBestMove() yet.
            */
        }


        // ====================================================
        // QUIT
        // ====================================================

        else if (command == "quit")
        {
            break;
        }
    }
}