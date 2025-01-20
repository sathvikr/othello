#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
#include <locale>
#include <vector>
#include "emulator/BoardManager.hpp"

void printBoard(uint64_t blackDisks, uint64_t whiteDisks) {
    std::cout << "\n  A B C D E F G H\n";
    for (int row = 0; row < 8; row++) {
        std::cout << row + 1 << " ";
        for (int col = 0; col < 8; col++) {
            int pos = row * 8 + col;
            uint64_t mask = 1ULL << pos;
            if (blackDisks & mask) std::cout << "● ";
            else if (whiteDisks & mask) std::cout << "○ ";
            else std::cout << ". ";
        }
        std::cout << row + 1 << "\n";
    }
    std::cout << "  A B C D E F G H\n\n";
}

std::string indexToAlgebraic(int index) {
    int row = index / 8;
    int col = index % 8;
    return std::string(1, char('A' + col)) + std::string(1, char('1' + row));
}

std::vector<std::string> getLegalMovesList(uint64_t legalMoves) {
    std::vector<std::string> moves;
    for (int i = 0; i < 64; i++) {
        if (legalMoves & (1ULL << i)) {
            moves.push_back(indexToAlgebraic(i));
        }
    }
    return moves;
}

int convertMove(const std::string& move) {
    if (move.length() != 2) return -1;
    
    char col = std::toupper(move[0], std::locale());
    char row = move[1];
    
    if (col < 'A' || col > 'H' || row < '1' || row > '8') return -1;
    
    int colIdx = col - 'A';
    int rowIdx = row - '1';
    
    return rowIdx * 8 + colIdx;
}

int main() {
    uint64_t blackDisks = 0x0000000810000000ULL;
    uint64_t whiteDisks = 0x0000001008000000ULL;
    bool isBlackTurn = true;
    int moveCount = 1;
    
    std::cout << "Welcome to Othello!\n";
    std::cout << "Enter moves in the format 'E4' (column A-H, row 1-8)\n";
    std::cout << "Type 'quit' to exit, 'moves' to see legal moves\n";
    std::cout << "● = Black, ○ = White\n\n";
    
    while (true) {
        printBoard(blackDisks, whiteDisks);
        
        uint64_t legalMoves = generateMoves(
            isBlackTurn ? blackDisks : whiteDisks,
            isBlackTurn ? whiteDisks : blackDisks
        );
        
        auto legalMovesList = getLegalMovesList(legalMoves);
        
        if (!legalMoves && !generateMoves(
            isBlackTurn ? whiteDisks : blackDisks,
            isBlackTurn ? blackDisks : whiteDisks)) {
            int blackCount = __builtin_popcountll(blackDisks);
            int whiteCount = __builtin_popcountll(whiteDisks);
            
            std::cout << "\nGame Over!\n";
            std::cout << "Final Score:\n";
            std::cout << "Black: " << blackCount << " disks\n";
            std::cout << "White: " << whiteCount << " disks\n";
            if (blackCount > whiteCount) std::cout << "Black wins!\n";
            else if (whiteCount > blackCount) std::cout << "White wins!\n";
            else std::cout << "It's a tie!\n";
            break;
        }
        
        if (!legalMoves) {
            std::cout << "\nMove " << moveCount << ": " 
                     << (isBlackTurn ? "Black" : "White") 
                     << " has no legal moves. Turn skipped.\n";
            isBlackTurn = !isBlackTurn;
            continue;
        }
        
        std::cout << "\nMove " << moveCount << ": " 
                  << (isBlackTurn ? "Black" : "White") << "'s turn\n";
        std::cout << "Legal moves: ";
        for (const auto& move : legalMovesList) {
            std::cout << move << " ";
        }
        std::cout << "\n";
        
        std::string moveStr;
        int moveIdx;
        uint64_t moveMask;
        bool validMove = false;
        
        do {
            std::cout << "Enter move: ";
            std::cin >> moveStr;
            
            if (moveStr == "quit") return 0;
            
            if (moveStr == "moves") {
                std::cout << "Legal moves: ";
                for (const auto& move : legalMovesList) {
                    std::cout << move << " ";
                }
                std::cout << "\n";
                continue;
            }
            
            moveIdx = convertMove(moveStr);
            if (moveIdx == -1) {
                std::cout << "Invalid format. Use column (A-H) and row (1-8), e.g., 'E4'\n";
                continue;
            }
            
            moveMask = 1ULL << moveIdx;
            if (!(legalMoves & moveMask)) {
                std::cout << "Illegal move. The move " << moveStr 
                         << " is not in the list of legal moves: ";
                for (const auto& move : legalMovesList) {
                    std::cout << move << " ";
                }
                std::cout << "\nTry again.\n";
                continue;
            }
            
            validMove = true;
            
        } while (!validMove);
        
        std::cout << "\nMove " << moveCount << ": " 
                  << (isBlackTurn ? "Black" : "White")
                  << " plays " << moveStr << "\n";
        
        if (isBlackTurn) {
            resolveMove(&blackDisks, &whiteDisks, moveIdx);
        } else {
            resolveMove(&whiteDisks, &blackDisks, moveIdx);
        }
        
        isBlackTurn = !isBlackTurn;
        moveCount++;
    }
    
    return 0;
}
