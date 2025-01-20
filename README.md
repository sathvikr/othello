# Othello Game Engine

## Overview
This is an Othello (Reversi) game engine implementation that handles game logic and board management.

## Functional Requirements

### Core Game Logic
- Maintain and manage an 8x8 game board state
- Track black and white disk positions
- Generate all legal moves for the current player
- Validate and execute player moves
- Apply disk flipping rules when moves are made
- Detect game end conditions

### Move Generation
- Calculate all valid moves for current player
- Return list of legal moves in algebraic notation (e.g. "E4")
- Support passing turn when no legal moves exist

### Interface Requirements
- Accept moves via UART serial interface
- Send board state and game information via UART
- Support standard Othello move notation (A1-H8)
- Display current player, legal moves, and game status

### Error Handling
- Validate all move inputs
- Reject illegal moves with appropriate error messages
- Handle edge cases like no legal moves available

## Technical Interface

### Input (UART)
- Move commands in algebraic notation (e.g. "E4")
- Control commands (e.g. "quit", "moves")

### Output (UART) 
- Current board state visualization
- Legal moves list
- Game status messages
- Error messages
- Final score and winner
