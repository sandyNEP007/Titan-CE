#include <iostream>
#include <string>

#include "board.h"
#include "move.h"

using namespace std;

Move parseMove(const string& s)
{
    int fromCol = s[0] - 'a';
    int fromRow = 8 - (s[1] - '0');

    int toCol = s[2] - 'a';
    int toRow = 8 - (s[3] - '0');

    return Move(fromRow, fromCol, toRow, toCol);
}

int main()
{
    Board board;

    bool whiteTurn = true;

    while (true)
    {
        board.display();

        if (board.isCheckmate(whiteTurn))
        {
            if (whiteTurn)
                cout << "\nCheckmate! Titan Wins!\n";
            else
                cout << "\nCheckmate! You Win!\n";

            break;
        }

        if (board.isStalemate(whiteTurn))
        {
            cout << "\nStalemate!\n";
            break;
        }

        if (whiteTurn)
        {
            string input;

            cout << "\nYour Move (e2e4): ";
            cin >> input;

            if (input == "exit")
                break;

            if (input.length() != 4)
            {
                cout << "Invalid Format!\n";
                continue;
            }

            Move move = parseMove(input);

            if (!board.isLegalMove(move))
            {
                cout << "Illegal Move!\n";
                continue;
            }

            board.makeMove(move);
        }
        else
        {
            cout << "\nTitan Thinking...\n";

           Move bestMove = board.findBestMove(4, whiteTurn);

            board.makeMove(bestMove);

            cout
                << "Titan Played: "
                << char('a' + bestMove.fromCol)
                << 8 - bestMove.fromRow
                << char('a' + bestMove.toCol)
                << 8 - bestMove.toRow
                << endl;
        }

        whiteTurn = !whiteTurn;
    }

    return 0;
}