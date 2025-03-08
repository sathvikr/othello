#include "BoardManager.hpp"
#include <iostream>
#include <iomanip>
#include <string>

// ANSI escape codes for colors
const std::string RESET = "\033[0m";
const std::string BLACK_PIECE = "\033[30;47m ● \033[0m";  // Black on white
const std::string WHITE_PIECE = "\033[37;40m ○ \033[0m";  // White on black
const std::string EMPTY_CELL = "\033[32;40m · \033[0m";   // Green dot on black
const std::string LEGAL_MOVE = "\033[33;40m □ \033[0m";   // Yellow square on black

BoardManager::BoardManager() {
    resetBoard();
}

BoardManager::~BoardManager() {
}

void BoardManager::resetBoard() {
    // Initial board setup
    playerBoard = 0x0000000810000000ULL;   // Black's initial pieces
    opponentBoard = 0x0000001008000000ULL;  // White's initial pieces
    isBlackTurn = true;  // Black moves first
    legalMoves = getLegalMoves();
}

void BoardManager::printBoard(bool showLegalMoves) {
    std::cout << "\n     A   B   C   D   E   F   G   H\n";
    std::cout << "   ┌───┬───┬───┬───┬───┬───┬───┬───┐\n";
    
    for (int row = 0; row < 8; row++) {
        std::cout << " " << (row + 1) << " │";
        for (int col = 0; col < 8; col++) {
            uint64_t mask = 1ULL << (row * 8 + col);
            if (playerBoard & mask) {
                std::cout << (isBlackTurn ? BLACK_PIECE : WHITE_PIECE) << "│";
            } else if (opponentBoard & mask) {
                std::cout << (isBlackTurn ? WHITE_PIECE : BLACK_PIECE) << "│";
            } else if (showLegalMoves && (legalMoves & mask)) {
                std::cout << LEGAL_MOVE << "│";
            } else {
                std::cout << EMPTY_CELL << "│";
            }
        }
        if (row < 7) {
            std::cout << "\n   ├───┼───┼───┼───┼───┼───┼───┼───┤\n";
        }
    }
    std::cout << "\n   └───┴───┴───┴───┴───┴───┴───┴───┘\n";
}

void BoardManager::printBb(uint64_t bb) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            std::cout << ((bb >> (row * 8 + col)) & 1);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

bool BoardManager::applyMove(uint8_t move) {
    if (!isLegalMove(move)) {
        return false;
    }

    uint64_t moveMask = 1ULL << move;
    uint64_t flips = 0;

    // Try all 8 directions
    for (int i = 0; i < 8; i++) {
        int8_t dir = DIRECTIONS[i];
        uint64_t mask = EDGE_MASKS[i];
        uint64_t possibleFlips = 0;
        uint64_t current = moveMask;

        // Move in the current direction while we see opponent pieces
        while ((current & mask) && (current = (dir > 0 ? current << dir : current >> -dir))) {
            if (current & opponentBoard) {
                possibleFlips |= current;
            } else if (current & playerBoard) {
                flips |= possibleFlips;
                break;
            } else {
                break;
            }
        }
    }

    if (flips == 0) {
        return false;
    }

    // Apply the move
    playerBoard |= moveMask | flips;
    opponentBoard &= ~flips;

    switchTurn();
    return true;
}

void BoardManager::switchTurn() {
    std::swap(playerBoard, opponentBoard);
    isBlackTurn = !isBlackTurn;
    legalMoves = getLegalMoves();
}

uint64_t BoardManager::getLegalMoves() {
    uint64_t moves = 0;
    uint64_t empty = ~(playerBoard | opponentBoard);

    // Try all 8 directions from each opponent piece
    for (int i = 0; i < 8; i++) {
        int8_t dir = DIRECTIONS[i];
        uint64_t mask = EDGE_MASKS[i];
        uint64_t candidates = opponentBoard & mask;
        
        if (dir > 0) {
            candidates = (candidates << dir) & empty;
            while (candidates) {
                moves |= candidates & empty;
                candidates = ((candidates & mask) << dir) & playerBoard;
            }
        } else {
            candidates = (candidates >> -dir) & empty;
            while (candidates) {
                moves |= candidates & empty;
                candidates = ((candidates & mask) >> -dir) & playerBoard;
            }
        }
    }

    return moves;
}

bool BoardManager::isLegalMove(uint8_t move) const {
    return (legalMoves & (1ULL << move)) != 0;
}

uint64_t BoardManager::getPlayerBoard() const {
    return playerBoard;
}

uint64_t BoardManager::getOpponentBoard() const {
    return opponentBoard;
}

bool BoardManager::isBlackPlayer() const {
    return isBlackTurn;
}