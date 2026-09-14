#ifndef MOVE_PICKER_H
#define MOVE_PICKER_H

#include "board.h"
#include <algorithm>
#include <vector>

class MovePicker
{
public:
    MovePicker(
        Board& board,
        std::vector<Move>& moves,
        const Move& ttMove,
        int depth
    );

    Move nextMove();

private:
    Board& board;
    int depth;

    Move ttMove;

    std::vector<Move> goodCaptures;
    std::vector<Move> badCaptures;
    std::vector<Move> goodQuiets;
    std::vector<Move> badQuiets;
    std::vector<Move> captures;
    std::vector<Move> quiets;

    int goodCaptureIndex;
    int goodQuietIndex;
    int badCaptureIndex;
    int badQuietIndex;
    bool ttMoveValid;

    enum Stage
    {
        MAIN_TT,
        CAPTURE_INIT,
        GOOD_CAPTURE,
        QUIET_INIT,
        GOOD_QUIET,
        BAD_CAPTURE,
        BAD_QUIET,
        DONE
    };

    Stage stage;

    bool ttMoveUsed;
};

#endif