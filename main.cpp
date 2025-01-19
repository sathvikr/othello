#include "emulator/BoardManager.hpp"
#include "emulator/BestMoveSelector.hpp"

#include <iostream>

int main() {
    BoardManager boardManager;

    while (true) {
        boardManager.printBoard();
        std::cout << "Enter move: ";
        uint8_t move;
        std::cin >> reinterpret_cast<int&>(move);
        std::cout << std::endl;
        std::cout << "APPLYING MOVE: " << static_cast<int>(move) << std::endl;
        boardManager.applyMove(move);
    }

    // while (true) {
        // std::cout << "Player board: " << boardManager.getPlayerBoard() << std::endl;
        // std::cout << "Opponent board: " << boardManager.getOpponentBoard() << std::endl;
        
        // uint64_t legalMoves = boardManager.getLegalMoves();

        // if (legalMoves == 0) {
        //     std::cout << "No legal moves found. Game over." << std::endl;
        //     break;
        // }

        // uint8_t bestMove = BestMoveSelector::selectBestMove(boardManager);

        // if (!boardManager.applyMove(bestMove)) {
        //     std::cout << "Invalid move. Game over." << std::endl;
        //     break;
        // }

        // std::cout << "Best move: " << bestMove << std::endl;
    // }

    return 0;
}