#ifndef BOARD_MANAGER_HPP
#define BOARD_MANAGER_HPP

#include <cstdint>

class BoardManager {
    private:
        uint64_t playerBoard;
        uint64_t opponentBoard;
        uint64_t legalMoves;
        bool isBlackTurn;
    public:
        const uint64_t NOT_LEFT_EDGE = 0xfefefefefefefefe;
        const uint64_t NOT_RIGHT_EDGE = 0x7f7f7f7f7f7f7f7f;
        const int8_t DIRECTIONS[8] = {8, -8, 1, -1, 9, 7, -7, -9};
        const uint64_t EDGE_MASKS[8] = {
            ~0ULL,          // N
            ~0ULL,          // S
            NOT_RIGHT_EDGE,   // E
            NOT_LEFT_EDGE,    // W
            NOT_RIGHT_EDGE,   // NE
            NOT_LEFT_EDGE,    // NW
            NOT_LEFT_EDGE,    // SE
            NOT_RIGHT_EDGE    // SW
        };

        BoardManager();
        ~BoardManager();

        void resetBoard();
        bool applyMove(uint8_t move);
        uint64_t getLegalMoves();
        uint64_t getPlayerBoard() const;
        uint64_t getOpponentBoard() const;
        void printBoard(bool showLegalMoves = true);
        void printBb(uint64_t bb);
        void switchTurn();
        bool isBlackPlayer() const;
        bool isLegalMove(uint8_t move) const;
};

#endif