#ifndef BOARD_MANAGER_HPP
#define BOARD_MANAGER_HPP

#include <cstdint>

class BoardManager {
    private:
        uint64_t playerBoard;
        uint64_t opponentBoard;
        uint64_t legalMoves;
    public:
        BoardManager();
        ~BoardManager();

        void initializeBoard();
        void resetBoard();
        bool applyMove(uint8_t move);
        uint64_t getLegalMoves();
        uint64_t getPlayerBoard();
        uint64_t getOpponentBoard();
};

#endif