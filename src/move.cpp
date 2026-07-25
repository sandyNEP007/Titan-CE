#include "move.h"
Move::Move(int fromRow, int fromCol, int toRow, int toCol)
{
    this->fromRow = fromRow;
    this->fromCol = fromCol;
    this->toRow = toRow;
    this->toCol = toCol;
}
Move::Move()
{
    fromRow = 0;
    fromCol = 0;
    toRow = 0;
    toCol = 0;
}