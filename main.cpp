#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
#include <locale>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include "emulator/BoardManager.hpp"
#include <fstream>
#include <sstream>

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

// Get random move from legal moves
int getRandomMove(uint64_t legalMoves) {
    std::vector<int> moves;
    for (int i = 0; i < 64; i++) {
        if (legalMoves & (1ULL << i)) {
            moves.push_back(i);
        }
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, moves.size() - 1);
    return moves[dis(gen)];
}

// Get current time in microseconds
int64_t getCurrentMicros() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

// Function to execute test based on function name
std::string executeTest(const std::string& functionName, const std::vector<std::string>& params) {
    std::stringstream result;
    
    if (functionName == "shift") {
        if (params.size() < 2) {
            return "Error: shift requires 2 parameters:\n"
                   "1. disks (hex): the board state to shift\n"
                   "2. dir (int): direction to shift (0-7)\n"
                   "Example: --test shift 0x0000000810000000 3";
        }
        try {
            uint64_t board = std::stoull(params[0].substr(2), nullptr, 16);  // skip "0x"
            int direction = std::stoi(params[1]);
            if (direction < 0 || direction >= NUM_DIRS) {
                return "Error: direction must be between 0 and 7";
            }
            uint64_t shifted = shift(board, direction);
            
            // Write directly to the log file in iverilog format
            std::ofstream logFile("test_results.log");
            logFile << "disks = " << std::hex << std::setfill('0') << std::setw(16) << std::nouppercase 
                   << board << ", dir = " << std::dec << direction 
                   << ", result = " << std::hex << std::setfill('0') << std::setw(16) 
                   << shifted;
            logFile.close();
            
            // Return the same format for display
            result << "disks = " << std::hex << std::setfill('0') << std::setw(16) << std::nouppercase 
                   << board << ", dir = " << std::dec << direction 
                   << ", result = " << std::hex << std::setfill('0') << std::setw(16) 
                   << shifted;
        } catch (const std::exception& e) {
            result << "Error: Invalid parameters for shift function";
        }
    } 
    else if (functionName == "generateMoves") {
        if (params.size() < 2) {
            return "Error: generateMoves requires 2 parameters:\n"
                   "1. myDisks (hex): current player's disk positions\n"
                   "2. oppDisks (hex): opponent's disk positions\n"
                   "Example: --test generateMoves 0x0000000810000000 0x0000001008000000";
        }
        try {
            uint64_t playerDisks = std::stoull(params[0], nullptr, 16);
            uint64_t opponentDisks = std::stoull(params[1], nullptr, 16);
            
            // Validate that disks don't overlap
            if (playerDisks & opponentDisks) {
                return "Error: player and opponent disks cannot overlap";
            }
            
            uint64_t moves = generateMoves(playerDisks, opponentDisks);
            result << "0x" << std::hex << std::uppercase << moves;
            
            // Also show the moves in algebraic notation for readability
            auto movesList = getLegalMovesList(moves);
            if (!movesList.empty()) {
                result << "\nLegal moves in algebraic notation: ";
                for (const auto& move : movesList) {
                    result << move << " ";
                }
            } else {
                result << "\nNo legal moves available";
            }
            
            // Print the current board for visualization
            result << "\n\nCurrent board state:";
            std::stringstream boardState;
            printBoard(playerDisks, opponentDisks);
            result << boardState.str();
        } catch (const std::exception& e) {
            result << "Error: Invalid parameters for generateMoves function";
        }
    }
    else if (functionName == "resolveMove") {
        if (params.size() < 3) {
            return "Error: resolveMove requires 3 parameters:\n"
                   "1. myDisks (hex): current player's disk positions\n"
                   "2. oppDisks (hex): opponent's disk positions\n"
                   "3. boardIdx: move position (either as 0-63 number or algebraic notation like 'E4')\n"
                   "Example: --test resolveMove 0x0000000810000000 0x0000001008000000 E4\n"
                   "     or --test resolveMove 0x0000000810000000 0x0000001008000000 28";
        }
        try {
            uint64_t playerDisks = std::stoull(params[0], nullptr, 16);
            uint64_t opponentDisks = std::stoull(params[1], nullptr, 16);
            
            // Validate that disks don't overlap
            if (playerDisks & opponentDisks) {
                return "Error: player and opponent disks cannot overlap";
            }
            
            int moveIndex;
            std::string moveStr = params[2];
            
            // Handle move input in both algebraic (e.g., "E4") and numeric (e.g., "28") formats
            if (moveStr.length() == 2 && std::isalpha(moveStr[0])) {
                moveIndex = convertMove(moveStr);
                if (moveIndex == -1) {
                    return "Error: Invalid algebraic move format. Use column (A-H) and row (1-8), e.g., 'E4'";
                }
            } else {
                moveIndex = std::stoi(moveStr);
                if (moveIndex < 0 || moveIndex >= 64) {
                    return "Error: Move index must be between 0 and 63";
                }
            }
            
            // Validate that the move is legal
            uint64_t legalMoves = generateMoves(playerDisks, opponentDisks);
            if (!(legalMoves & (1ULL << moveIndex))) {
                auto movesList = getLegalMovesList(legalMoves);
                std::string moveNotation = indexToAlgebraic(moveIndex);
                return "Error: " + moveNotation + " is not a legal move. Legal moves are: " + 
                       (movesList.empty() ? "none" : [&movesList]() {
                           std::string moves;
                           for (const auto& m : movesList) moves += m + " ";
                           return moves;
                       }());
            }
            
            uint64_t newPlayerDisks = playerDisks;
            uint64_t newOpponentDisks = opponentDisks;
            resolveMove(&newPlayerDisks, &newOpponentDisks, moveIndex);
            
            result << "Move applied: " << indexToAlgebraic(moveIndex) << " (index " << moveIndex << ")\n\n";
            result << "Player disks after move: 0x" << std::hex << std::uppercase << newPlayerDisks << "\n";
            result << "Opponent disks after move: 0x" << newOpponentDisks << "\n\n";
            result << "Board state after move:";
            std::stringstream boardState;
            printBoard(newPlayerDisks, newOpponentDisks);
            result << boardState.str();
        } catch (const std::exception& e) {
            result << "Error: Invalid parameters for resolveMove function";
        }
    }
    else {
        result << "Error: Unknown test function '" << functionName << "'\n";
        result << "Available functions:\n";
        result << "1. shift: Shifts a board state in a given direction\n";
        result << "2. generateMoves: Generates legal moves for a player\n";
        result << "3. resolveMove: Applies a move and returns the new board state";
    }
    
    return result.str();
}

// Test logging function - no longer needed for shift tests
void logTestResult(const std::string& functionName, const std::string& params, const std::string& result) {
    if (functionName != "shift") {
        // Standard logging for other functions
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%c", std::localtime(&now_time));
        
        std::ofstream logFile("test_results.log", std::ios::app);
        logFile << "=== Test Execution: " << time_str << "\n";
        logFile << "Function: " << functionName << "\n";
        logFile << "Parameters: " << params << "\n";
        logFile << "Result: " << result << "\n\n";
        logFile.close();
    }
}

int main(int argc, char* argv[]) {
    // Handle test flag
    if (argc > 2 && std::string(argv[1]) == "--test") {
        std::string functionName = argv[2];
        std::vector<std::string> params;
        for (int i = 3; i < argc; i++) {
            params.push_back(argv[i]);
        }
        
        std::string result = executeTest(functionName, params);
        
        // Only call logTestResult for non-shift tests
        if (functionName != "shift") {
            std::string paramStr = argc > 3 ? argv[3] : "none";
            logTestResult(functionName, paramStr, result);
        }
        return 0;
    }

    enum class GameMode {
        HUMAN_VS_HUMAN,
        HUMAN_VS_COMPUTER,
        COMPUTER_VS_COMPUTER
    };

    GameMode mode = GameMode::HUMAN_VS_HUMAN;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--vs-computer") {
            mode = GameMode::HUMAN_VS_COMPUTER;
        } else if (arg == "--computer-vs-computer") {
            mode = GameMode::COMPUTER_VS_COMPUTER;
        }
    }

    uint64_t blackDisks = 0x0000000810000000ULL;
    uint64_t whiteDisks = 0x0000001008000000ULL;
    bool isBlackTurn = true;
    int moveCount = 1;
    
    std::cout << "Welcome to Othello!\n";
    switch (mode) {
        case GameMode::HUMAN_VS_HUMAN:
            std::cout << "Mode: Human vs Human\n";
            break;
        case GameMode::HUMAN_VS_COMPUTER:
            std::cout << "Mode: Human (Black) vs Computer (White)\n";
            break;
        case GameMode::COMPUTER_VS_COMPUTER:
            std::cout << "Mode: Computer vs Computer\n";
            break;
    }
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
        
        int moveIdx;
        std::string moveStr;
        bool isComputerMove = (mode == GameMode::COMPUTER_VS_COMPUTER) || 
                             (mode == GameMode::HUMAN_VS_COMPUTER && !isBlackTurn);

        if (isComputerMove) {
            int64_t startTime = getCurrentMicros();
            moveIdx = getRandomMove(legalMoves);
            moveStr = indexToAlgebraic(moveIdx);
            int64_t endTime = getCurrentMicros();
            std::cout << "Computer plays " << moveStr << " (took " 
                     << (endTime - startTime) << " microseconds)\n";
        } else {
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
        }

        int64_t startTime = getCurrentMicros();
        if (isBlackTurn) {
            resolveMove(&blackDisks, &whiteDisks, moveIdx);
        } else {
            resolveMove(&whiteDisks, &blackDisks, moveIdx);
        }
        int64_t endTime = getCurrentMicros();
        
        std::cout << "\nMove " << moveCount << ": " 
                  << (isBlackTurn ? "Black" : "White")
                  << " plays " << moveStr 
                  << " (move execution took " << (endTime - startTime) << " microseconds)\n";
        
        isBlackTurn = !isBlackTurn;
        moveCount++;
    }
    
    return 0;
}
