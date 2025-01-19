#ifndef MOVE_EVALUATOR_HPP
#define MOVE_EVALUATOR_HPP

#include <cstdint>
#include "BoardManager.hpp"

class MoveEvaluator {
    public:
        static int8_t evaluateMove(uint8_t move, BoardManager board);
};

#endif