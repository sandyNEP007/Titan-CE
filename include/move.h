#ifndef MOVE_H
#define MOVE_H

#include <string>

class Move
{
public:
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;

    char promotion;

    Move();

    Move(int fromRow, int fromCol, int toRow, int toCol, char promotion = '\0');
    bool operator==(const Move& other) const;
    static Move parseMove(const std::string& input);
};

#endif