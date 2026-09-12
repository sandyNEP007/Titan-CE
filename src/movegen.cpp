#include<iostream>
#include "movegen.h"
#include "board.h"

std::vector<Move> MoveGenerator::generateLegalMoves(Board& board)
{
    std::vector<Move> legalMoves;

    const bool white = board.isWhiteTurn();

    // ------------------------------------------------------------
    // Piece indexes from Board::PieceIndex
    // ------------------------------------------------------------
    const int MY_PAWN   = white ? Board::WP : Board::BP;
    const int MY_KNIGHT = white ? Board::WN : Board::BN;
    const int MY_BISHOP = white ? Board::WB : Board::BB;
    const int MY_ROOK   = white ? Board::WR : Board::BR;
    const int MY_QUEEN  = white ? Board::WQ : Board::BQ;
    const int MY_KING   = white ? Board::WK : Board::BK;

    const int EN_PAWN   = white ? Board::BP : Board::WP;
    const int EN_KNIGHT = white ? Board::BN : Board::WN;
    const int EN_BISHOP = white ? Board::BB : Board::WB;
    const int EN_ROOK   = white ? Board::BR : Board::WR;
    const int EN_QUEEN  = white ? Board::BQ : Board::WQ;
    const int EN_KING   = white ? Board::BK : Board::WK;

    const uint64_t ownOcc   = white ? board.whiteOccupancy : board.blackOccupancy;
    const uint64_t enemyOcc = white ? board.blackOccupancy : board.whiteOccupancy;
    const uint64_t occupied = board.allOccupancy;

   
    auto popLSB = [](uint64_t& bb) -> int
    {
        int sq = __builtin_ctzll(bb);
        bb &= bb - 1;
        return sq;
    };

    auto bit = [](int sq) -> uint64_t
    {
        return 1ULL << sq;
    };

    auto rowOf = [](int sq) -> int
    {
        return sq / 8;
    };

    auto colOf = [](int sq) -> int
    {
        return sq % 8;
    };

   
    auto bishopAttacks = [&](int sq, uint64_t occ) -> uint64_t
    {
        uint64_t attacks = 0;

        const int r = rowOf(sq);
        const int c = colOf(sq);

        const int dirs[4][2] =
        {
            {-1, -1},
            {-1,  1},
            { 1, -1},
            { 1,  1}
        };

        for (const auto& d : dirs)
        {
            int nr = r + d[0];
            int nc = c + d[1];

            while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8)
            {
                int nsq = nr * 8 + nc;
                attacks |= bit(nsq);

                if (occ & bit(nsq))
                    break;

                nr += d[0];
                nc += d[1];
            }
        }

        return attacks;
    };

    auto rookAttacks = [&](int sq, uint64_t occ) -> uint64_t
    {
        uint64_t attacks = 0;

        const int r = rowOf(sq);
        const int c = colOf(sq);

        const int dirs[4][2] =
        {
            {-1, 0},
            { 1, 0},
            { 0,-1},
            { 0, 1}
        };

        for (const auto& d : dirs)
        {
            int nr = r + d[0];
            int nc = c + d[1];

            while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8)
            {
                int nsq = nr * 8 + nc;
                attacks |= bit(nsq);

                if (occ & bit(nsq))
                    break;

                nr += d[0];
                nc += d[1];
            }
        }

        return attacks;
    };

    auto isSquareAttackedBB =
        [&](int sq,
            bool byWhite,
            uint64_t occ,
            const uint64_t* pieces) -> bool
    {
        const int pawn   = byWhite ? Board::WP : Board::BP;
        const int knight = byWhite ? Board::WN : Board::BN;
        const int bishop = byWhite ? Board::WB : Board::BB;
        const int rook   = byWhite ? Board::WR : Board::BR;
        const int queen  = byWhite ? Board::WQ : Board::BQ;
        const int king   = byWhite ? Board::WK : Board::BK;

        // Pawn attacks.
        if (board.pawnAttacks[byWhite ? 0 : 1][sq] & pieces[pawn])
            return true;

        // Knight attacks.
        if (board.knightAttacks[sq] & pieces[knight])
            return true;

        // King attacks.
        if (board.kingAttacks[sq] & pieces[king])
            return true;

        // Bishop / Queen.
        if (bishopAttacks(sq, occ) & (pieces[bishop] | pieces[queen]))
            return true;

        // Rook / Queen.
        if (rookAttacks(sq, occ) & (pieces[rook] | pieces[queen]))
            return true;

        return false;
    };


    uint64_t kingBB = board.pieceBB[MY_KING];

    if (!kingBB)
        return legalMoves;

    const int kingSq = __builtin_ctzll(kingBB);
    const int kingRow = rowOf(kingSq);
    const int kingCol = colOf(kingSq);


    uint64_t checkers = 0;

    // Pawn checker.
    checkers |= board.pawnAttacks[white ? 1 : 0][kingSq]
                & board.pieceBB[EN_PAWN];

    // Knight checker.
    checkers |= board.knightAttacks[kingSq]
                & board.pieceBB[EN_KNIGHT];

    // King checker.
    checkers |= board.kingAttacks[kingSq]
                & board.pieceBB[EN_KING];

    // Sliding checkers.
    uint64_t bishopLike =
        board.pieceBB[EN_BISHOP] |
        board.pieceBB[EN_QUEEN];

    uint64_t rookLike =
        board.pieceBB[EN_ROOK] |
        board.pieceBB[EN_QUEEN];

    checkers |= bishopAttacks(kingSq, occupied) & bishopLike;
    checkers |= rookAttacks(kingSq, occupied) & rookLike;

    const int numberOfCheckers = __builtin_popcountll(checkers);

   
    uint64_t pinned = 0;
    uint64_t pinMasks[64];

    for (int i = 0; i < 64; ++i)
        pinMasks[i] = ~0ULL;

    const int allDirs[8][2] =
    {
        {-1,-1}, {-1,0}, {-1,1},
        { 0,-1},          { 0,1},
        { 1,-1}, { 1,0},  { 1,1}
    };

    for (const auto& d : allDirs)
    {
        const bool diagonal =
            (d[0] != 0 && d[1] != 0);

        const int enemySlider =
            diagonal ? EN_BISHOP : EN_ROOK;

        int r = kingRow + d[0];
        int c = kingCol + d[1];

        int friendlySq = -1;

        while (r >= 0 && r < 8 && c >= 0 && c < 8)
        {
            int sq = r * 8 + c;

            if (!(occupied & bit(sq)))
            {
                r += d[0];
                c += d[1];
                continue;
            }

            if (ownOcc & bit(sq))
            {
                // First friendly piece on the ray.
                if (friendlySq == -1)
                {
                    friendlySq = sq;
                    r += d[0];
                    c += d[1];
                    continue;
                }

                // Second friendly piece blocks the pin.
                break;
            }

            // Enemy piece.
            if (friendlySq != -1)
            {
                const bool isSlider =
                    (board.pieceBB[enemySlider] & bit(sq)) ||
                    (board.pieceBB[EN_QUEEN] & bit(sq));

                if (isSlider)
                {
                    pinned |= bit(friendlySq);

                    uint64_t mask = 0;

                    int pr = rowOf(kingSq);
                    int pc = colOf(kingSq);

                    while (true)
                    {
                        int psq = pr * 8 + pc;
                        mask |= bit(psq);

                        if (psq == sq)
                            break;

                        pr += d[0];
                        pc += d[1];
                    }

                    pinMasks[friendlySq] = mask;
                }
            }

            break;
        }
    }

   
    uint64_t kingTargets =
        board.kingAttacks[kingSq] & ~ownOcc;

    while (kingTargets)
    {
        const int toSq = popLSB(kingTargets);

        // Remove our king from its old square.
        uint64_t newOcc =
            occupied ^ bit(kingSq);

        // Remove captured enemy piece, if any.
        if (enemyOcc & bit(toSq))
            newOcc ^= bit(toSq);

        // Put king on destination.
        newOcc |= bit(toSq);

        // Copy piece bitboards because a capture may remove an enemy.
        uint64_t tempPieces[12];

        for (int i = 0; i < 12; ++i)
            tempPieces[i] = board.pieceBB[i];

        tempPieces[MY_KING] &= ~bit(kingSq);
        tempPieces[MY_KING] |= bit(toSq);

        // Remove captured enemy piece.
        if (enemyOcc & bit(toSq))
        {
            for (int i = EN_PAWN; i <= EN_KING; ++i)
                tempPieces[i] &= ~bit(toSq);
        }

        if (!isSquareAttackedBB(
                toSq,
                !white,
                newOcc,
                tempPieces))
        {
            legalMoves.emplace_back(
                kingRow,
                kingCol,
                rowOf(toSq),
                colOf(toSq)
            );
        }
    }

    if (numberOfCheckers >= 2)
        return legalMoves;

    uint64_t evasionMask = ~0ULL;

    if (numberOfCheckers == 1)
    {
        const int checkerSq = __builtin_ctzll(checkers);

        evasionMask = bit(checkerSq);

        const bool checkerIsBishopLike =
            (board.pieceBB[EN_BISHOP] |
             board.pieceBB[EN_QUEEN]) & bit(checkerSq);

        const bool checkerIsRookLike =
            (board.pieceBB[EN_ROOK] |
             board.pieceBB[EN_QUEEN]) & bit(checkerSq);

        if (checkerIsBishopLike || checkerIsRookLike)
        {
            int kr = kingRow;
            int kc = kingCol;

            const int cr = rowOf(checkerSq);
            const int cc = colOf(checkerSq);

            int dr = (cr > kr) ? 1 : (cr < kr ? -1 : 0);
            int dc = (cc > kc) ? 1 : (cc < kc ? -1 : 0);

            int r = kr + dr;
            int c = kc + dc;

            while (r != cr || c != cc)
            {
                evasionMask |= bit(r * 8 + c);

                r += dr;
                c += dc;
            }
        }
    }


    uint64_t pawns = board.pieceBB[MY_PAWN];

    while (pawns)
    {
        const int fromSq = popLSB(pawns);

        const int r = rowOf(fromSq);
        const int c = colOf(fromSq);

        const int direction = white ? -1 : 1;
        const int promotionRow = white ? 0 : 7;
        const int startRow = white ? 6 : 1;

        const bool pawnPinned = (pinned & bit(fromSq)) != 0;
        const int oneRow = r + direction;


        if (oneRow >= 0 && oneRow < 8)
        {
            const int toSq = oneRow * 8 + c;
            

            if (!(occupied & bit(toSq)) &&
                (evasionMask & bit(toSq))&&
            (!pawnPinned || (pinMasks[fromSq] & bit(toSq))))
            {
                if (oneRow == promotionRow)
                {
                    legalMoves.emplace_back(r,c,oneRow,c,'Q');
                    legalMoves.emplace_back(r,c,oneRow,c,'R');
                    legalMoves.emplace_back(r,c,oneRow,c,'B');
                    legalMoves.emplace_back(r,c,oneRow,c,'N');
                }
                else
                {
                    legalMoves.emplace_back(r,c,oneRow,c);

                    // Two-square push.
                    if (r == startRow)
                    {
                        const int twoRow = r + 2 * direction;
                        const int twoSq = twoRow * 8 + c;

                        if (!(occupied & bit(twoSq)) &&
                            (evasionMask & bit(twoSq))&&
                        (!pawnPinned || (pinMasks[fromSq] & bit(toSq))))
                        {
                            legalMoves.emplace_back(r,c,twoRow,c);
                        }
                    }
                }
            }
        }

        
       for (int dc : {-1, 1})
        {
            const int nr = r + direction;
            const int nc = c + dc;

            if (nr < 0 || nr >= 8 ||
                nc < 0 || nc >= 8)
                continue;

            const int toSq = nr * 8 + nc;

            if (!(enemyOcc & bit(toSq)))
                continue;

            if (!(evasionMask & bit(toSq)))
                continue;

            if (pawnPinned && !(pinMasks[fromSq] & bit(toSq)))
                continue;

            if (nr == promotionRow)
            {
                legalMoves.emplace_back(r, c, nr, nc, 'Q');
                legalMoves.emplace_back(r, c, nr, nc, 'R');
                legalMoves.emplace_back(r, c, nr, nc, 'B');
                legalMoves.emplace_back(r, c, nr, nc, 'N');
            }
            else
            {
                legalMoves.emplace_back(r, c, nr, nc);
            }
        }
        const int epSq = board.getEnPassantSquare();

        if (epSq != -1)
        {
            const int epRow = rowOf(epSq);
            const int epCol = colOf(epSq);

            if (epRow == r + direction &&
                std::abs(epCol - c) == 1)
            {
                // EP cannot be accepted blindly when it exposes the
                // king to a rook/bishop/queen.
                uint64_t newOcc = occupied;

                newOcc &= ~bit(fromSq);
                newOcc &= ~bit(epSq);

                const int actualCapturedPawnSq =
                    r * 8 + epCol;

                newOcc &= ~bit(actualCapturedPawnSq);
                newOcc |= bit(epSq);

                uint64_t tempPieces[12];

                for (int i = 0; i < 12; ++i)
                    tempPieces[i] = board.pieceBB[i];

                tempPieces[MY_PAWN] &= ~bit(fromSq);
                tempPieces[MY_PAWN] |= bit(epSq);
                tempPieces[EN_PAWN] &= ~bit(actualCapturedPawnSq);

                if (!isSquareAttackedBB(
                        kingSq,
                        !white,
                        newOcc,
                        tempPieces))
                {
                    // In check, EP must also resolve the check.
                    if (numberOfCheckers == 0 ||
                        (evasionMask & bit(epSq)) ||
                        (evasionMask & bit(actualCapturedPawnSq)))
                    {
                        legalMoves.emplace_back(
                            r,
                            c,
                            epRow,
                            epCol
                        );
                    }
                }
            }
        }
    }


    uint64_t knights = board.pieceBB[MY_KNIGHT];

    while (knights)
    {
        const int fromSq = popLSB(knights);

        uint64_t targets =
            board.knightAttacks[fromSq] & ~ownOcc;

        targets &= evasionMask;

        // A knight cannot move along a pin.
        if (pinned & bit(fromSq))
            targets = 0;

        while (targets)
        {
            const int toSq = popLSB(targets);

            legalMoves.emplace_back(
                rowOf(fromSq),
                colOf(fromSq),
                rowOf(toSq),
                colOf(toSq)
            );
        }
    }

    
    uint64_t bishops = board.pieceBB[MY_BISHOP];

    while (bishops)
    {
        const int fromSq = popLSB(bishops);

        uint64_t targets =
            bishopAttacks(fromSq, occupied) & ~ownOcc;

        targets &= evasionMask;

        if (pinned & bit(fromSq))
            targets &= pinMasks[fromSq];

        while (targets)
        {
            const int toSq = popLSB(targets);

            legalMoves.emplace_back(
                rowOf(fromSq),
                colOf(fromSq),
                rowOf(toSq),
                colOf(toSq)
            );
        }
    }

    uint64_t rooks = board.pieceBB[MY_ROOK];

    while (rooks)
    {
        const int fromSq = popLSB(rooks);

        uint64_t targets =
            rookAttacks(fromSq, occupied) & ~ownOcc;

        targets &= evasionMask;

        if (pinned & bit(fromSq))
            targets &= pinMasks[fromSq];

        while (targets)
        {
            const int toSq = popLSB(targets);

            legalMoves.emplace_back(
                rowOf(fromSq),
                colOf(fromSq),
                rowOf(toSq),
                colOf(toSq)
            );
        }
    }

    // ------------------------------------------------------------
    // Queens
    // ------------------------------------------------------------

    uint64_t queens = board.pieceBB[MY_QUEEN];

    while (queens)
    {
        const int fromSq = popLSB(queens);

        uint64_t targets =
            (bishopAttacks(fromSq, occupied) |
             rookAttacks(fromSq, occupied))
            & ~ownOcc;

        targets &= evasionMask;

        if (pinned & bit(fromSq))
            targets &= pinMasks[fromSq];

        while (targets)
        {
            const int toSq = popLSB(targets);

            legalMoves.emplace_back(
                rowOf(fromSq),
                colOf(fromSq),
                rowOf(toSq),
                colOf(toSq)
            );
        }
    }

    if (numberOfCheckers == 0)
    {
        Move kingSideCastle(
            kingRow,
            kingCol,
            kingRow,
            kingCol + 2
        );

        Move queenSideCastle(
            kingRow,
            kingCol,
            kingRow,
            kingCol - 2
        );

        if (board.isValidCastle(kingSideCastle))
            legalMoves.push_back(kingSideCastle);

        if (board.isValidCastle(queenSideCastle))
            legalMoves.push_back(queenSideCastle);
    }

    return legalMoves;
}
//Generating pseudo-legal move of pawn
