#include "BoardManager.hpp"

uint64_t shift(uint64_t disks, int dir) {
    static const uint64_t MASKS[] = {
        0x7F7F7F7F7F7F7F7FULL, 0x007F7F7F7F7F7F7FULL, 0xFFFFFFFFFFFFFFFFULL, 0x00FEFEFEFEFEFEFEULL,
        0xFEFEFEFEFEFEFEFEULL, 0xFEFEFEFEFEFEFE00ULL, 0xFFFFFFFFFFFFFFFFULL, 0x7F7F7F7F7F7F7F00ULL
    };
    static const uint64_t LSHIFTS[] = { 0, 0, 0, 0, 1, 9, 8, 7 };
    static const uint64_t RSHIFTS[] = { 1, 9, 8, 7, 0, 0, 0, 0 };

    if (dir < NUM_DIRS / 2) {
        return (disks >> RSHIFTS[dir]) & MASKS[dir];
    } else {
        return (disks << LSHIFTS[dir]) & MASKS[dir];
    }
}

uint64_t generateMoves(uint64_t myDisks, uint64_t oppDisks) {
    int dir;
    uint64_t x;
    uint64_t emptyCells = ~(myDisks | oppDisks);
    uint64_t legalMoves = 0;

    for (dir = 0; dir < NUM_DIRS; dir++) {
        x = shift(myDisks, dir) & oppDisks;

        x |= shift(x, dir) & oppDisks;
        x |= shift(x, dir) & oppDisks;
        x |= shift(x, dir) & oppDisks;
        x |= shift(x, dir) & oppDisks;
        x |= shift(x, dir) & oppDisks;

        legalMoves |= shift(x, dir) & emptyCells;
    }

    return legalMoves;
}

void resolveMove(uint64_t *myDisks, uint64_t *oppDisks, int boardIdx) {
    int dir;
    uint64_t x, boundingDisk;
    uint64_t newDisk = 1ULL << boardIdx;
    uint64_t capturedDisks = 0;

    *myDisks |= newDisk;

    for (dir = 0; dir < NUM_DIRS; dir++) {
        x = shift(newDisk, dir) & *oppDisks;

        x |= shift(x, dir) & *oppDisks;
        x |= shift(x, dir) & *oppDisks;
        x |= shift(x, dir) & *oppDisks;
        x |= shift(x, dir) & *oppDisks;
        x |= shift(x, dir) & *oppDisks;

        boundingDisk = shift(x, dir) & *myDisks;
        capturedDisks |= (boundingDisk ? x : 0);
    }

    *myDisks ^= capturedDisks;
    *oppDisks ^= capturedDisks;
}