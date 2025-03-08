# Welcome to Othello!

```
   ██████  ████████ ██   ██ ███████ ██      ██       ██████     
  ██    ██    ██    ██   ██ ██      ██      ██      ██    ██    
  ██    ██    ██    ███████ █████   ██      ██      ██    ██    
  ██    ██    ██    ██   ██ ██      ██      ██      ██    ██    
  ██    ██    ██    ██   ██ ██      ██      ██      ██    ██    
   ██████     ██    ██   ██ ███████ ███████ ███████  ██████     
```

This project implements the classic game of Othello (Reversi) using bitboard techniques in C++ with a parallel SystemVerilog hardware implementation, featuring an advanced AI opponent.

## Playing the Game

### Building and Running
```bash
make        # Build the C++ implementation
make run    # Run the game
make clean  # Clean build files
```

### Game Commands
- Enter moves using coordinates (e.g., `E4`, `C5`)
- Type `help` to see available commands
- Type `legal` to toggle legal move indicators
- Type `quit` to exit the game

## Implementation Details

### Bitboard Representation
The game uses bitboards (64-bit integers) to represent the board state. Each bit represents one square on the 8x8 board:

```
Position indices (hex):    Binary representation example:
00 01 02 03 04 05 06 07   0 0 0 0 0 0 0 0
08 09 0A 0B 0C 0D 0E 0F   0 0 0 0 0 0 0 0
10 11 12 13 14 15 16 17   0 0 0 1 0 0 0 0  <- Black piece at 13
18 19 1A 1B 1C 1D 1E 1F   0 0 0 0 1 0 0 0  <- White piece at 1C
20 21 22 23 24 25 26 27   0 0 0 0 0 0 0 0
28 29 2A 2B 2C 2D 2E 2F   0 0 0 0 0 0 0 0
30 31 32 33 34 35 36 37   0 0 0 0 0 0 0 0
38 39 3A 3B 3C 3D 3E 3F   0 0 0 0 0 0 0 0
```

### Key Data Structures
- `playerBoard`: Current player's pieces (1 = piece present)
- `opponentBoard`: Opponent's pieces (1 = piece present)
- `legalMoves`: Valid moves for current player (1 = legal move)

### Move Generation
Legal moves are calculated by:
1. Finding empty squares
2. For each empty square, checking all 8 directions
3. A move is legal if it would flip at least one opponent piece

Example of finding legal moves in one direction:
```
Initial state:        After placing at E3:
· · · · · · · ·      · · · · · · · ·
· · · · · · · ·      · · · · · · · ·
· · · · □ · · ·      · · · · ● · · ·
· · · ○ ● · · ·      · · · ○ ● · · ·
· · · ● ○ · · ·      · · · ● ○ · · ·
· · · · · · · ·      · · · · · · · ·
· · · · · · · ·      · · · · · · · ·
· · · · · · · ·      · · · · · · · ·

□ = Legal move
● = Black piece
○ = White piece
```

### Edge Handling
Special masks prevent wrapping around board edges:
```c++
const uint64_t NOT_LEFT_EDGE  = 0xfefefefefefefefe;  // All columns except A
const uint64_t NOT_RIGHT_EDGE = 0x7f7f7f7f7f7f7f7f;  // All columns except H
```

## AI Implementation

### Minimax with Alpha-Beta Pruning
The AI uses a minimax algorithm with alpha-beta pruning, optimized for hardware acceleration:

```cpp
int minimax(BoardState state, int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) return evaluatePosition(state);
    
    uint64_t moves = state.getLegalMoves();
    if (!moves) return evaluatePosition(state);
    
    int bestValue = maximizing ? INT_MIN : INT_MAX;
    while (moves) {
        uint8_t pos = __builtin_ctzll(moves);  // Get next move
        BoardState newState = state.applyMove(pos);
        
        int value = minimax(newState, depth - 1, alpha, beta, !maximizing);
        bestValue = maximizing ? std::max(value, bestValue) 
                              : std::min(value, bestValue);
        
        if (maximizing) alpha = std::max(alpha, bestValue);
        else beta = std::min(beta, bestValue);
        
        if (beta <= alpha) break;  // Pruning
        moves &= (moves - 1);      // Clear lowest set bit
    }
    return bestValue;
}
```

### Position Evaluation
The evaluation function considers multiple factors:
```cpp
int evaluatePosition(const BoardState& state) {
    return pieceCount * PIECE_WEIGHT +
           mobility * MOBILITY_WEIGHT +
           cornerControl * CORNER_WEIGHT +
           edgeStability * EDGE_WEIGHT +
           potentialMobility * POTENTIAL_WEIGHT;
}
```

Weights are tuned using machine learning on a database of professional games.

### Opening Book
```
                  Initial Position
                        ↓
         E4 → D3 → C5 → F6 → F5 → F4
         ↓
    D3 → C3 → D6 → C4 → F4 → E3 → F5
    ↓
    C4 → E3 → F4 → C5 → B4 → A5
```

## Parallel Move Processing

### Software Implementation
Eight directions are processed simultaneously using bit manipulation:

```cpp
uint64_t generateMoves() {
    uint64_t moves = 0;
    uint64_t empty = ~(playerBoard | opponentBoard);
    
    // Process all 8 directions in parallel using bit shifts
    uint64_t candidates = opponentBoard & NOT_EDGE_MASKS[0];
    moves |= shift<NORTH>(candidates) & empty;
    moves |= shift<SOUTH>(candidates) & empty;
    moves |= shift<EAST>(candidates) & empty;
    moves |= shift<WEST>(candidates) & empty;
    moves |= shift<NORTHEAST>(candidates) & empty;
    moves |= shift<NORTHWEST>(candidates) & empty;
    moves |= shift<SOUTHEAST>(candidates) & empty;
    moves |= shift<SOUTHWEST>(candidates) & empty;
    
    return moves & validMoveMask;
}
```

### Hardware Implementation
The hardware implementation processes all directions truly in parallel using dedicated hardware modules for each direction. Each direction checker operates simultaneously, with results combined to determine legal moves.

### Parallel Move Validation
Each potential move is validated simultaneously using parallel direction validators. The system checks all eight directions at once, with dedicated hardware for each direction check. Results are combined through a fast reduction network to determine final move validity.

## Hardware Acceleration

### Overall Architecture
```
                                  ┌─────────────────┐
                     ┌──────────►│  Move Generator │
                     │           └────────┬────────┘
                     │                    │
┌──────────┐    ┌───┴────┐        ┌─────▼─────┐
│  Input   │───►│ Board  │◄───────┤   Move    │
│ Handler  │    │ State  │        │ Validator  │
└──────────┘    └───┬────┘        └─────▲─────┘
                    │                    │
                    │           ┌────────┴────────┐
                    └──────────►│ Position Score  │
                               └─────────────────┘
```

### Move Generator Pipeline
```
Stage 1: Direction Calculation
┌─────────────┐   ┌─────────────┐   ┌─────────────┐
│   North     │   │   South     │   │    East     │
│  Shifter    │   │  Shifter    │   │  Shifter    │
└─────┬───────┘   └─────┬───────┘   └─────┬───────┘
      │                 │                 │
      └─────────┬───────┴────────┬───────┘
                │                │
         ┌──────▼──────┐  ┌──────▼──────┐
         │   Edge      │  │   Empty      │
         │   Mask      │  │   Check      │
         └──────┬──────┘  └──────┬──────┘
                │               │
                └───────┬───────┘
                       ▼
              Stage 2: Validation
                ┌─────────┐
                │  Move   │
                │ Filter  │
                └────┬────┘
                     │
              Stage 3: Output
                ┌─────────┐
                │  Legal  │
                │ Moves   │
                └─────────┘
```

### AI Processing Unit
```
┌───────────────────────────────────────────────┐
│               Evaluation Engine               │
├───────────┬───────────┬───────────┬──────────┤
│  Piece    │ Mobility  │  Corner   │  Edge    │
│ Counter   │ Analyzer  │ Control   │ Stability│
└─────┬─────┴─────┬─────┴─────┬─────┴────┬─────┘
      │           │           │          │
┌─────▼─────┬─────▼─────┬─────▼────┬────▼─────┐
│   Weight   │   Weight  │  Weight  │  Weight  │
│     W1     │     W2    │    W3    │    W4    │
└─────┬─────┴─────┬─────┴─────┬────┴─────┬────┘
      │           │           │          │
      └───────────┼───────────┼──────────┘
                  │           │
            ┌─────▼─────┬────▼─────┐
            │  Adder    │  Final   │
            │   Tree    │  Score   │
            └───────────┴──────────┘
```

### Parallel Move Validation
```
┌─────────────┐
│ Input Move  │
└──────┬──────┘
       │
   ┌───▼───┐
   │ Split │
   └┬─┬─┬─┬┘
┌──┘ │ │ └──┐
│    │ │    │
▼    ▼ ▼    ▼
N    S E    W   Direction
│    │ │    │   Validators
├────┼─┼────┤
│    │ │    │
▼    ▼ ▼    ▼
┌──┐ │ │ ┌──┐
│  └─┼─┼─┘  │
   ┌─▼─▼─┐
   │Merge │
   └──┬───┘
      ▼
┌──────────┐
│  Valid   │
│  Move    │
└──────────┘
```

## Performance Characteristics

### Pipeline Timing
```
Clock Cycle │  1  │  2  │  3  │  4  │  5  │
────────────┼─────┼─────┼─────┼─────┼─────┤
Direction   │ M1  │ M2  │ M3  │ M4  │ M5  │
Validation  │     │ M1  │ M2  │ M3  │ M4  │
Output      │     │     │ M1  │ M2  │ M3  │
────────────┴─────┴─────┴─────┴─────┴─────┘
M1-M5 = Different moves being processed
```

## License
This project is licensed under the MIT License - see the LICENSE file for details.