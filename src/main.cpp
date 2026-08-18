
#include <iostream>
#include <string>
#include <chrono>

#include "board.h"
#include "move.h"
#include "engine_search.h"
#include "evaluation.h"


void printMove(const Move& move)
{
    char fromFile = 'a' + move.fromCol;
    char fromRank = '8' - move.fromRow;

    char toFile = 'a' + move.toCol;
    char toRank = '8' - move.toRow;

    std::cout << fromFile << fromRank
              << toFile << toRank;

    if (move.promotion != '\0')
    {
        std::cout << "=" << move.promotion;
    }
}


int main()
{
    Board board;

    std::cout << "=============================\n";
    std::cout << "        TITAN-CE\n";
    std::cout << "=============================\n";
    std::cout << "Titan = White\n";
    std::cout << "You   = Black\n";
    std::cout << "Enter moves like: e7e5\n";
    std::cout << "Type quit to exit.\n\n";


    while (true)
    {
        board.display();


        // =========================================
        // TITAN'S TURN
        // =========================================

        if (board.isWhiteTurn())
        {
            std::cout << "\nTitan is thinking...\n";

            auto start =
                std::chrono::high_resolution_clock::now();

            Move bestMove = EngineSearch::findBestMove(board, 6);

            auto end =
                std::chrono::high_resolution_clock::now();

            double seconds =
                std::chrono::duration<double>(end - start).count();


            // Make Titan's move
            board.makeMove(bestMove);

            std::cout << "\nTitan plays: ";
            printMove(bestMove);
            std::cout << "\n";


            std::cout << "Nodes: "
                      << EngineSearch::nodes
                      << "\n";

            std::cout << "Cutoffs: "
                      << EngineSearch::cutoffs
                      << "\n";

            std::cout << "QNodes: "
                      << EngineSearch::qNodes
                      << "\n";

            std::cout << "QCutoffs: "
                      << EngineSearch::qCutoffs
                      << "\n";

            std::cout << "TT Hits: "
                      << EngineSearch::ttHits
                      << "\n";

            std::cout << "Time: "
                      << seconds
                      << " seconds\n";


            if (seconds > 0.0)
            {
                double nps =
                    static_cast<double>(
                        EngineSearch::nodes
                    ) / seconds;

                std::cout << "NPS: "
                          << static_cast<long long>(nps)
                          << "\n";
            }


            // Check whether Black is in check
            if (board.isKingInCheck(false))
            {
                std::cout << "\nCHECK!\n";
            }

            continue;
        }


        // =========================================
        // HUMAN'S TURN
        // =========================================

        std::string input;

        std::cout << "\nYour move: ";
        std::cin >> input;


        if (input == "quit")
        {
            std::cout << "Game ended.\n";
            break;
        }


        Move humanMove;

        try
        {
            humanMove = Move::parseMove(input);
        }
        catch (...)
        {
            std::cout << "Invalid move format.\n";
            std::cout << "Use something like e7e5.\n";
            continue;
        }


        // =========================================
        // VALIDATE HUMAN MOVE DIRECTLY
        // =========================================

        if (!board.isValidMove(humanMove))
        {
            std::cout << "Illegal move.\n";
            continue;
        }


        // =========================================
        // MAKE HUMAN MOVE
        // =========================================

        board.makeMove(humanMove);

        std::cout << "You played: ";
        printMove(humanMove);
        std::cout << "\n";


        // Check whether Titan is in check
        if (board.isKingInCheck(true))
        {
            std::cout << "\nCHECK!\n";
        }
    }


    return 0;
}

