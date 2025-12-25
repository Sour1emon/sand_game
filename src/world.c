#include "world.h"
#include "block.h"
#include "consts.h"
#include "rng.h"
#include "state.h"
#include <stdint.h>
#include <stdlib.h>

Block *getBlock(unsigned int x, unsigned int y) {
  if (x >= WORLD_WIDTH || y >= WORLD_HEIGHT) {
    return NULL;
  }
  return &_state.world[y][x];
}

bool setBlock(unsigned int x, unsigned int y, Block block) {
  if (x >= WORLD_WIDTH || y >= WORLD_HEIGHT) {
    return false;
  }
  _state.world[y][x] = block;
  return true;
}

static inline void swap(Block *a, Block *b) {
  Block temp = *a;
  *a = *b;
  *b = temp;
}

static inline bool isPassibleBlock(Block *block) {
  return block != NULL && IsPassible(block);
}

// Ceiling divide a by b
// https://stackoverflow.com/a/2745086
#define CEIL_DIV(a, b) (1 + (((a) - 1) / (b)))

#define UINT64_BITS (sizeof(uint64_t) * 8)

enum {
  // Calculate the size of the bitmap
  BITMAP_SIZE = CEIL_DIV(WORLD_WIDTH * WORLD_HEIGHT, UINT64_BITS),
};

bool hasCellProcessed(uint64_t processed[BITMAP_SIZE], unsigned int x,
                      unsigned int y) {
  unsigned int a = y * WORLD_WIDTH + x;
  unsigned int idx = a / UINT64_BITS;
  unsigned int rem = a % UINT64_BITS;
  uint64_t val = (processed[idx] >> rem) & 0x1;
  return val == 1;
}

void setCellProcessed(uint64_t processed[BITMAP_SIZE], unsigned int x,
                      unsigned int y, bool value) {
  unsigned int a = y * WORLD_WIDTH + x;
  unsigned int idx = a / UINT64_BITS;
  unsigned int rem = a % UINT64_BITS;
  // Cast value to 64-bit before shifting to avoid undefined behavior when
  // rem >= 32 on platforms where int is 32 bits.
  processed[idx] = (processed[idx] & ~(1ULL << rem)) | ((uint64_t)value << rem);
}

static bool trySwapWithCandidates(Block *block, int x, int y, Block *first,
                                  bool firstPassible, int firstDx, int firstDy,
                                  Block *second, bool secondPassible,
                                  int secondDx, int secondDy,
                                  uint64_t processed[BITMAP_SIZE],
                                  bool markBelow) {
  if (!firstPassible && !secondPassible) {
    return false;
  }

  bool useFirst =
      firstPassible && secondPassible ? pcg32_bool() : firstPassible;
  Block *target = useFirst ? first : second;
  int destX = x + (useFirst ? firstDx : secondDx);
  int destY = y + (useFirst ? firstDy : secondDy);

  swap(target, block);
  setCellProcessed(processed, destX, destY, true);
  if (markBelow) {
    setCellProcessed(processed, x, y - 1, true);
  }
  setCellProcessed(processed, x, y, true);
  return true;
}

void worldTick() {

  // Bitmap
  uint64_t processed[BITMAP_SIZE] = {false};

  // Handle blocks that fall down
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      if (hasCellProcessed(processed, x, y)) {
        continue;
      }

      Block *block = getBlock(x, y);
      Block *below = getBlock(x, y - 1);

      if (block == NULL || below == NULL) {
        continue;
      }

      // Skip gases because they are handled later
      if (IsGas(block)) {
        continue;
      }

      if (y > 0 && HasGravity(block)) {
        // Try falling straight down first
        if (IsPassible(below)) {
          swap(block, below);
          setCellProcessed(processed, x, y - 1, true);
          setCellProcessed(processed, x, y, true);
          continue;
        }

        // Check to see if the block below is water
        if (!IsFluid(block) && IsFluid(below)) {
          // Check which blocks are passible
          // Order: above -> side -> lower diagonal -> upper diagonal -> swap
          // (last resort)
          // TODO: Find a better last resort method because this might teleport
          // blocks up too far

          Block *above = getBlock(x, y + 1);
          if (trySwapWithCandidates(block, x, y, above, isPassibleBlock(above),
                                    0, 1, NULL, false, 0, 0, processed, true)) {
            continue;
          }

          Block *leftBlock = getBlock(x - 1, y - 1);
          Block *rightBlock = getBlock(x + 1, y - 1);
          if (trySwapWithCandidates(block, x, y, leftBlock,
                                    isPassibleBlock(leftBlock), -1, -1,
                                    rightBlock, isPassibleBlock(rightBlock), 1,
                                    -1, processed, true)) {
            continue;
          }

          leftBlock = getBlock(x - 1, y - 2);
          rightBlock = getBlock(x + 1, y - 2);
          if (trySwapWithCandidates(block, x, y, leftBlock,
                                    isPassibleBlock(leftBlock), -1, -2,
                                    rightBlock, isPassibleBlock(rightBlock), 1,
                                    -2, processed, true)) {
            continue;
          }

          leftBlock = getBlock(x - 1, y);
          rightBlock = getBlock(x + 1, y);
          if (trySwapWithCandidates(block, x, y, leftBlock,
                                    isPassibleBlock(leftBlock), -1, 0,
                                    rightBlock, isPassibleBlock(rightBlock), 1,
                                    0, processed, true)) {
            continue;
          }

          // Last resort swap
          swap(below, block);
          setCellProcessed(processed, x, y - 1, true);
          setCellProcessed(processed, x, y, true);
          continue;
        }
      }

      // If can't fall straight, try sliding diagonally
      if (CanSlide(block)) {
        Block *leftBlock = getBlock(x - 1, y - 1);
        bool isLeftPassible =
            (IsPassible(leftBlock) || IsFluid(leftBlock)) &&
            (IsPassible(getBlock(x - 1, y)) || IsFluid(getBlock(x - 1, y))) &&
            leftBlock->type != block->type;
        Block *rightBlock = getBlock(x + 1, y - 1);
        bool isRightPassible =
            (IsPassible(rightBlock) || IsFluid(rightBlock)) &&
            (IsPassible(getBlock(x + 1, y)) || IsFluid(getBlock(x + 1, y))) &&
            rightBlock->type != block->type;
        if (trySwapWithCandidates(block, x, y, leftBlock, isLeftPassible, -1,
                                  -1, rightBlock, isRightPassible, 1, -1,
                                  processed, false)) {
          continue;
        }
      }

      // Move fluids side to side
      // TODO: Fix weird fluid movement logic where going one direction it will
      // clump together but the other it will break apart
      if (IsFluid(block)) {
        Block *leftBlock = getBlock(x - 1, y);
        bool isLeftPassible = IsPassible(leftBlock);

        Block *rightBlock = getBlock(x + 1, y);
        bool isRightPassible = IsPassible(rightBlock);
        // Make sure there is a place to move before doing other checks
        if (isLeftPassible || isRightPassible) {

          if (isLeftPassible && !isRightPassible) {
            // Only the left is passible
            block->movementDir = DIR_LEFT;
          } else if (!isLeftPassible && isRightPassible) {
            // Only the right is passible
            block->movementDir = DIR_RIGHT;
          } else if (block->movementDir == DIR_NONE) {
            // Randomly generate a new fluid direction
            block->movementDir = pcg32_bool() ? DIR_LEFT : DIR_RIGHT;
          }

          if (block->movementDir == DIR_LEFT) {
            Direction leftDir = leftBlock->movementDir;
            Direction currentDir = block->movementDir;
            swap(leftBlock, block);
            block->movementDir = leftDir;
            leftBlock->movementDir = currentDir;
            setCellProcessed(processed, x - 1, y, true);
            setCellProcessed(processed, x, y, true);
            continue;
          } else if (block->movementDir == DIR_RIGHT) {
            Direction rightDir = rightBlock->movementDir;
            Direction currentDir = block->movementDir;
            swap(rightBlock, block);
            block->movementDir = rightDir;
            rightBlock->movementDir = currentDir;
            setCellProcessed(processed, x + 1, y, true);
            setCellProcessed(processed, x, y, true);
            continue;
          }
        }
      }
    }
  }

  // Handle blocks that float upward
  // TODO: Allow smoke to move diagonally
  for (int y = WORLD_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      if (hasCellProcessed(processed, x, y)) {
        continue;
      }

      Block *block = getBlock(x, y);
      Block *above = getBlock(x, y + 1);

      // Skip non gases since they were handled earlier
      if (!IsGas(block)) {
        continue;
      }

      if (IsGas(block) && IsPassible(above) && block->type != above->type) {
        swap(block, above);
        setCellProcessed(processed, x, y + 1, true);
        setCellProcessed(processed, x, y, true);
      }
    }
  }
}
