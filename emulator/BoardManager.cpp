#include "BoardManager.hpp"

BoardManager::BoardManager() {
    resetBoard();
}

BoardManager::~BoardManager() {
}

void BoardManager::resetBoard() {
    playerBoard = 0x0000000810000000;  // Initial position
    opponentBoard = 0x0000001008000000;
    legalMoves = 0x0000001020080000;
}

bool BoardManager::applyMove(uint8_t move) {
    // TODO: Implement move application logic
    return true;
}