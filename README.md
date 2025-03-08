# Othello Implementation with Bitboards and Hardware Design

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
The SystemVerilog implementation processes all directions truly in parallel:

```systemverilog
module move_generator (
    input  logic [63:0] player_board,
    input  logic [63:0] opponent_board,
    output logic [63:0] legal_moves
);
    logic [63:0] north_moves, south_moves, east_moves, west_moves;
    logic [63:0] ne_moves, nw_moves, se_moves, sw_moves;
    
    // Parallel direction modules
    direction_checker #(.DIRECTION(NORTH)) north_check (
        .player_board(player_board),
        .opponent_board(opponent_board),
        .moves(north_moves)
    );
    
    // [Similar modules for other 7 directions]
    
    // Combine results
    assign legal_moves = north_moves | south_moves | east_moves | west_moves |
                        ne_moves | nw_moves | se_moves | sw_moves;
endmodule

// Individual direction checker
module direction_checker #(
    parameter int DIRECTION = 8  // Number of positions to shift
) (
    input  logic [63:0] player_board,
    input  logic [63:0] opponent_board,
    output logic [63:0] moves
);
    logic [63:0] candidates;
    logic [63:0] empty = ~(player_board | opponent_board);
    logic [63:0] edge_mask;
    
    // Generate edge mask based on direction
    always_comb begin
        case (DIRECTION)
            NORTH:     edge_mask = 64'hFFFFFFFFFFFFFFFF;
            NORTHEAST: edge_mask = 64'h7F7F7F7F7F7F7F7F;
            // [Other directions...]
        endcase
    end
    
    // Pipeline stages for timing optimization
    always_ff @(posedge clk) begin
        // Stage 1: Find opponent pieces
        candidates <= opponent_board & edge_mask;
        
        // Stage 2: Shift and check empty squares
        moves <= shift_direction(candidates, DIRECTION) & empty;
        
        // Stage 3: Validate moves
        moves <= moves & valid_move_mask;
    end
endmodule
```

### Parallel Move Validation
Each potential move is validated simultaneously:

```systemverilog
module move_validator (
    input  logic [63:0] move_bitmap,
    input  logic [63:0] player_board,
    input  logic [63:0] opponent_board,
    output logic [63:0] valid_moves
);
    logic [63:0] valid_masks [0:7];  // One for each direction
    
    genvar i;
    generate
        for (i = 0; i < 8; i++) begin : direction_validators
            direction_validator #(
                .DIRECTION(i)
            ) validator (
                .move_bitmap(move_bitmap),
                .player_board(player_board),
                .opponent_board(opponent_board),
                .valid_mask(valid_masks[i])
            );
        end
    endgenerate
    
    // Combine results - a move is valid if it's valid in any direction
    always_comb begin
        valid_moves = '0;
        for (int i = 0; i < 8; i++)
            valid_moves |= valid_masks[i];
    end
endmodule
```

## Hardware Acceleration

### Pipelined Move Generation
The hardware implementation uses a 3-stage pipeline:
1. **Direction Calculation Stage**
   - All 8 directions processed in parallel
   - Each direction uses dedicated shift network
   ```systemverilog
   // Example of shift network for one direction
   module shift_network #(parameter DIR = NORTH) (
       input  logic [63:0] in_board,
       output logic [63:0] out_board
   );
       always_comb begin
           case (DIR)
               NORTH: out_board = {in_board[55:0], 8'b0};
               SOUTH: out_board = {8'b0, in_board[63:8]};
               // [Other directions...]
           endcase
       end
   endmodule
   ```

2. **Move Validation Stage**
   - Parallel validation of all potential moves
   - Uses look-ahead for opponent piece flipping

3. **Result Combination Stage**
   - Combines results from all directions
   - Applies final validity checks

### Performance Metrics
- Move generation: 1 cycle latency
- Move validation: 2 cycles latency
- Total pipeline depth: 3 cycles
- Maximum throughput: 1 move/cycle

## AI Hardware Acceleration

### Parallel Position Evaluation
```systemverilog
module position_evaluator (
    input  logic [63:0] player_board,
    input  logic [63:0] opponent_board,
    output logic [15:0] score
);
    // Parallel evaluation of multiple factors
    logic [7:0] piece_count, mobility, corners, edges;
    
    piece_counter   pc(.board(player_board), .count(piece_count));
    mobility_calc   mc(.board(player_board), .mobility(mobility));
    corner_analyzer ca(.board(player_board), .score(corners));
    edge_analyzer   ea(.board(player_board), .score(edges));
    
    // Weighted combination
    always_comb
        score = piece_count * PIECE_WEIGHT +
                mobility   * MOBILITY_WEIGHT +
                corners   * CORNER_WEIGHT +
                edges     * EDGE_WEIGHT;
endmodule
```


## License
This project is licensed under the MIT License - see the LICENSE file for details.