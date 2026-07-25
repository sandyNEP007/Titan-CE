#ifndef MOVE_H
#define MOVE_H
class Move{
    public:
    Move();
    Move(int fromRow, int fromCol, int toRow, int toCol);
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;

};
#endif