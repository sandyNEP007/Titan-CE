#include "move.h"

Move::Move()
{
    fromRow = -1;
    fromCol = -1;
    toRow = -1;
    toCol = -1;

    promotion = '\0';
}

Move::Move(
    int fromRow,
    int fromCol,
    int toRow,
    int toCol,
    char promotion)
{
    this->fromRow = fromRow;
    this->fromCol = fromCol;
    this->toRow = toRow;
    this->toCol = toCol;

    this->promotion = promotion;
}

Move Move::parseMove(const std::string& input)
{
    int fromCol = input[0] - 'a';
    int fromRow = 8 - (input[1] - '0');

    int toCol = input[2] - 'a';
    int toRow = 8 - (input[3] - '0');

    char promotion = '\0';

    if (input.length() >= 5)
    {
        promotion = input[4];
    }

    return Move(
        fromRow,
        fromCol,
        toRow,
        toCol,
        promotion
    );
}
bool Move::operator==(const Move& other) const
{
    return fromRow == other.fromRow &&
           fromCol == other.fromCol &&
           toRow == other.toRow &&
           toCol == other.toCol &&
           promotion == other.promotion;
}