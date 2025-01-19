#include "BoardManager.hpp"

#include <iostream>

BoardManager::BoardManager() {
    resetBoard();
    isBlackTurn = true;
}

BoardManager::~BoardManager() {
}

void BoardManager::resetBoard() {
    playerBoard = 0x0000000810000000;
    opponentBoard = 0x0000001008000000;
    legalMoves = 0x0000001020080000;
    isBlackTurn = true;
}

void BoardManager::switchTurn() {
    std::swap(playerBoard, opponentBoard);
    isBlackTurn = !isBlackTurn;
}

bool BoardManager::isBlackPlayer() const {
    return isBlackTurn;
}

void BoardManager::printBb(uint64_t bb) {
    for (int i = 56; i >= 0; i -= 8) {
        for (int j = 7; j >= 0; j--) {
            std::cout << ((bb >> (i + j)) & 1);
        }
        std::cout << '\n';
    }
}

bool BoardManager::applyMove(uint8_t move) {
    uint64_t moveMask = 1ULL << move;
    std::cout << "Applying move: " << static_cast<int>(move) << std::endl;
    std::cout << "Initial board:" << std::endl;
    printBoard();
    std::cout << "Player board:" << std::endl;
    printBb(playerBoard);
    std::cout << "Opponent board:" << std::endl;
    printBb(opponentBoard);
    
    for (int i = 0; i < 8; i++) {
        uint64_t check = moveMask, possibleFlips = 0;
        std::cout << "Direction " << i << ": " << std::endl;

        printBb(check);
        std::cout << "Player board:" << std::endl;
        printBb(playerBoard);
        std::cout << "Opponent board:" << std::endl;
        printBb(opponentBoard);
        
        while ((check = (i % 2 == 0 ? (check << DIRECTIONS[i]) : (check >> -DIRECTIONS[i]))) & opponentBoard) {
            possibleFlips |= check;
            std::cout << "  Found opponent piece at position:\n";
            printBb(check);
            std::cout << "  Current possible flips:\n";
            printBb(possibleFlips);
        }
        
        if (check & playerBoard) {
            std::cout << "  Found player piece at end, applying flips" << std::endl;
            playerBoard |= moveMask | possibleFlips;
            opponentBoard &= ~possibleFlips;
            std::cout << "  New board:" << std::endl;
            printBoard();
        } else {
            std::cout << "  No player piece found at end, discarding flips" << std::endl;
        }
    }    
    
    std::cout << "Final board:" << std::endl;
    printBoard();
    switchTurn();
    return true;
}

void BoardManager::printBoard() {
    std::cout << "  8 7 6 5 4 3 2 1" << std::endl;
    std::cout << " ┌────────────────┐" << std::endl;
    for (int i = 63; i >= 0; i--) {
        if (i % 8 == 7) {
            std::cout << (1 + i/8) << "│";
        }
        if ((playerBoard >> i) & 1) {
            std::cout << (isBlackTurn ? "x " : "o ");
        } else if ((opponentBoard >> i) & 1) {
            std::cout << (isBlackTurn ? "o " : "x ");
        } else {
            std::cout << ". ";
        }
        if (i % 8 == 0) {
            std::cout << "│" << (1 + i/8) << std::endl;
        }
    }
    std::cout << " └────────────────┘" << std::endl;
    std::cout << "  8 7 6 5 4 3 2 1" << std::endl;
}