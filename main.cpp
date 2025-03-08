#include "emulator/BoardManager.hpp"
#include "emulator/BestMoveSelector.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <iomanip>

// ANSI escape codes for colors
const std::string RESET = "\033[0m";
const std::string BLACK_PIECE = "\033[30;47m ● \033[0m";  // Black on white
const std::string WHITE_PIECE = "\033[37;40m ○ \033[0m";  // White on black
const std::string EMPTY_CELL = "\033[32;40m · \033[0m";   // Green dot on black
const std::string LEGAL_MOVE = "\033[33;40m □ \033[0m";   // Yellow square on black

// Rainbow colors
const std::string RED = "\033[38;5;196m";
const std::string ORANGE = "\033[38;5;214m";
const std::string YELLOW = "\033[38;5;226m";
const std::string GREEN = "\033[38;5;46m";
const std::string BLUE = "\033[38;5;33m";
const std::string PURPLE = "\033[38;5;129m";

void printWelcome() {
    std::cout << "\n";
    std::cout << RED    << "   ██████  ████████ ██   ██ ███████ ██      ██       ██████  \n";
    std::cout << ORANGE << "  ██    ██    ██    ██   ██ ██      ██      ██      ██    ██ \n";
    std::cout << YELLOW << "  ██    ██    ██    ███████ █████   ██      ██      ██    ██ \n";
    std::cout << GREEN  << "  ██    ██    ██    ██   ██ ██      ██      ██      ██    ██ \n";
    std::cout << BLUE   << "  ██    ██    ██    ██   ██ ██      ██      ██      ██    ██ \n";
    std::cout << PURPLE << "   ██████     ██    ██   ██ ███████ ███████ ███████  ██████  \n" << RESET;
}

// Convert coordinate (e.g., "E4") to move number (0-63)
uint8_t coordToMove(const std::string& coord) {
    if (coord.length() != 2) return 255;
    
    char col = std::toupper(coord[0]);
    char row = coord[1];
    
    if (col < 'A' || col > 'H' || row < '1' || row > '8') return 255;
    
    int x = col - 'A';
    int y = row - '1';
    
    return y * 8 + x;
}

// Convert move number to coordinate string
std::string moveToCoord(uint8_t move) {
    int x = move % 8;
    int y = move / 8;
    return std::string(1, char('A' + x)) + char('1' + y);
}

// Count bits set in a 64-bit integer (piece count)
int countPieces(uint64_t board) {
    int count = 0;
    while (board) {
        count += board & 1;
        board >>= 1;
    }
    return count;
}

void printHelp() {
    std::cout << "\nOthello Commands:\n"
              << "  - Enter moves using coordinates (e.g., E4)\n"
              << "  - Type 'help' to see this message\n"
              << "  - Type 'quit' to exit the game\n"
              << "  - Type 'legal' to see legal moves\n\n"
              << "Board notation:\n"
              << "  " << BLACK_PIECE << " = Black piece\n"
              << "  " << WHITE_PIECE << " = White piece\n"
              << "  " << EMPTY_CELL << " = Empty cell\n"
              << "  " << LEGAL_MOVE << " = Legal move\n\n";
}

void printScore(const BoardManager& board) {
    int blackCount = countPieces(board.isBlackPlayer() ? board.getPlayerBoard() : board.getOpponentBoard());
    int whiteCount = countPieces(board.isBlackPlayer() ? board.getOpponentBoard() : board.getPlayerBoard());
    
    std::cout << "\nScore: Black: " << blackCount << "  White: " << whiteCount << "\n";
}

int main() {
    BoardManager boardManager;
    std::string input;
    bool showLegalMoves = true;

    printWelcome();
    std::cout << "\nWelcome to the classic game of strategy!\n";
    printHelp();

    while (true) {
        std::cout << "\n";
        boardManager.printBoard(showLegalMoves);
        printScore(boardManager);
        
        std::cout << "\n" << (boardManager.isBlackPlayer() ? "Black" : "White") << "'s turn\n";
        std::cout << "Enter move (or 'help', 'quit', 'legal'): ";
        
        std::getline(std::cin, input);
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        if (input == "quit") {
            std::cout << "Thanks for playing!\n";
            break;
        } else if (input == "help") {
            printHelp();
            continue;
        } else if (input == "legal") {
            showLegalMoves = !showLegalMoves;
            continue;
        }

        uint8_t move = coordToMove(input);
        if (move == 255) {
            std::cout << "Invalid move format. Please use coordinates like 'E4'.\n";
            continue;
        }

        if (!boardManager.applyMove(move)) {
            std::cout << "Invalid move. Please try again.\n";
            continue;
        }

        uint64_t legalMoves = boardManager.getLegalMoves();
        if (legalMoves == 0) {
            std::cout << "\nGame Over! No more legal moves.\n";
            boardManager.printBoard(false);
            printScore(boardManager);
            break;
        }
    }

    return 0;
}