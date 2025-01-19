#ifndef BEST_MOVE_SELECTOR_HPP
#define BEST_MOVE_SELECTOR_HPP

#include <cstdint>
#include "BoardManager.hpp"

class BestMoveSelector {
    public:
        static uint8_t selectBestMove(BoardManager board);
};

#endif