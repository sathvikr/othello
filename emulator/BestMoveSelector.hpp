#ifndef BEST_MOVE_SELECTOR_HPP
#define BEST_MOVE_SELECTOR_HPP

#include <cstdint>

class BestMoveSelector {
    public:
        static uint8_t selectBestMove(uint64_t legalMoves, uint64_t playerBoard, uint64_t opponentBoard);
};

#endif