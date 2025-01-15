#include "BoardManager.hpp"
#include "BestMoveSelector.hpp"

#include <iostream>

int main() {
    BoardManager boardManager;
    boardManager.initializeBoard();

    while (true) {
        std::cout << "Player board: " << boardManager.getPlayerBoard() << std::endl;
        std::cout << "Opponent board: " << boardManager.getOpponentBoard() << std::endl;
        
        uint64_t legalMoves = boardManager.getLegalMoves();

        if (legalMoves == 0) {
            std::cout << "No legal moves found. Game over." << std::endl;
            break;
        }

        uint8_t bestMove = BestMoveSelector::selectBestMove(legalMoves, boardManager.getPlayerBoard(), boardManager.getOpponentBoard());

        if (!boardManager.applyMove(bestMove)) {
            std::cout << "Invalid move. Game over." << std::endl;
            break;
        }

        std::cout << "Best move: " << bestMove << std::endl;
    }
    
    return 0;
}