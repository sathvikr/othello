#ifndef BOARD_MANAGER_HPP
#define BOARD_MANAGER_HPP

#include <cstdint>
#include <cassert>

uint64_t shift(uint64_t disks, int dir);

static const int NUM_DIRS = 8;

uint64_t generateMoves(uint64_t myDisks, uint64_t oppDisks);

void resolveMove(uint64_t *myDisks, uint64_t *oppDisks, int boardIdx);

#endif
