#include "world.h"
#include "block.h"
#include "consts.h"
#include "rng.h"
#include "state.h"
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

void worldTick() {

  bool processed[WORLD_HEIGHT][WORLD_WIDTH] = {false};

  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      if (processed[y][x]) {
        continue;
      }
      Block *block = getBlock(x, y);
      Block *below = getBlock(x, y - 1);
      if (y > 0 && HasGravity(block->type) && below != NULL) {
        // Try falling straight down first
        if (IsPassible(below->type)) {
          swap(block, below);
          processed[y - 1][x] = true;
          processed[y][x] = true;
          continue;
        }

        // Check to see if the block below is water
        if (!IsFluid(block->type) && IsFluid(below->type)) {
          // Check which blocks are passible
          // Order: above -> side -> lower diagonal -> upper diagonal -> swap
          // (last resort)
          // TODO: Find a better last resort method because this might teleport
          // blocks up too far

          // Check the block above the current one to see if it is passible
          Block *above = getBlock(x, y + 1);
          if (above != NULL && IsPassible(above->type)) {
            swap(below, block);
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          }

          // TODO: Move this code into a new function

          // Check if the blocks next to the fluid are passible
          Block *leftBlock = getBlock(x - 1, y - 1);
          bool isLeftPassible =
              leftBlock != NULL ? IsPassible(leftBlock->type) : false;

          Block *rightBlock = getBlock(x + 1, y - 1);
          bool isRightPassible =
              rightBlock != NULL ? IsPassible(rightBlock->type) : false;

          if (isLeftPassible && isRightPassible) {
            bool slideLeft = pcg32_bool();
            swap(slideLeft ? leftBlock : rightBlock, block);
            processed[y - 1][x + (slideLeft ? -1 : 1)] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          } else if (isLeftPassible) {
            swap(leftBlock, block);
            processed[y - 1][x - 1] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          } else if (isRightPassible) {
            swap(rightBlock, block);
            processed[y - 1][x + 1] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          }

          // Check if the lower diagonals are passible
          leftBlock = getBlock(x - 1, y - 2);
          isLeftPassible =
              leftBlock != NULL ? IsPassible(leftBlock->type) : false;

          rightBlock = getBlock(x + 1, y - 2);
          isRightPassible =
              rightBlock != NULL ? IsPassible(rightBlock->type) : false;

          if (isLeftPassible && isRightPassible) {
            bool slideLeft = pcg32_bool();
            swap(slideLeft ? leftBlock : rightBlock, block);
            processed[y - 2][x + (slideLeft ? -1 : 1)] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          } else if (isLeftPassible) {
            swap(leftBlock, block);
            processed[y - 2][x - 1] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          } else if (isRightPassible) {
            swap(rightBlock, block);
            processed[y - 2][x + 1] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          }

          // Check if the upper diagonals are passible
          leftBlock = getBlock(x - 1, y);
          isLeftPassible =
              leftBlock != NULL ? IsPassible(leftBlock->type) : false;

          rightBlock = getBlock(x + 1, y);
          isRightPassible =
              rightBlock != NULL ? IsPassible(rightBlock->type) : false;

          if (isLeftPassible && isRightPassible) {
            bool slideLeft = pcg32_bool();
            swap(slideLeft ? leftBlock : rightBlock, block);
            processed[y][x + (slideLeft ? -1 : 1)] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          } else if (isLeftPassible) {
            swap(leftBlock, block);
            processed[y][x - 1] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          } else if (isRightPassible) {
            swap(rightBlock, block);
            processed[y][x + 1] = true;
            processed[y - 1][x] = true;
            processed[y][x] = true;
            continue;
          }

          // Last resort swap
          swap(below, block);
          processed[y - 1][x] = true;
          processed[y][x] = true;
          continue;
        }
      }

      // If can't fall straight, try sliding diagonally
      if (CanSlide(block->type)) {
        Block *leftBlock = getBlock(x - 1, y - 1);
        bool isLeftPassible = leftBlock != NULL
                                  ? IsPassible(leftBlock->type) &&
                                        IsPassible(_state.world[y][x - 1].type)
                                  : false;
        Block *rightBlock = getBlock(x + 1, y - 1);
        bool isRightPassible = rightBlock != NULL
                                   ? IsPassible(rightBlock->type) &&
                                         IsPassible(_state.world[y][x + 1].type)
                                   : false;
        if (isLeftPassible && isRightPassible) {
          bool slideLeft = pcg32_bool();
          swap(slideLeft ? leftBlock : rightBlock, block);
          processed[y - 1][x + (slideLeft ? -1 : 1)] = true;
          processed[y][x] = true;
          continue;
        } else if (isLeftPassible) {
          swap(leftBlock, block);
          processed[y - 1][x - 1] = true;
          processed[y][x] = true;
          continue;
        } else if (isRightPassible) {
          swap(rightBlock, block);
          processed[y - 1][x + 1] = true;
          processed[y][x] = true;
          continue;
        }
      }

      // Move fluids side to side
      // TODO: Fix weird fluid movement logic where going one direction it will
      // clump together but the other it will break apart
      if (IsFluid(block->type)) {
        Block *leftBlock = getBlock(x - 1, y);
        bool isLeftPassible =
            leftBlock != NULL ? IsPassible(leftBlock->type) : false;

        Block *rightBlock = getBlock(x + 1, y);
        bool isRightPassible =
            rightBlock != NULL ? IsPassible(rightBlock->type) : false;
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
            processed[y][x - 1] = true;
            processed[y][x] = true;
            continue;
          } else if (block->movementDir == DIR_RIGHT) {
            Direction rightDir = rightBlock->movementDir;
            Direction currentDir = block->movementDir;
            swap(rightBlock, block);
            block->movementDir = rightDir;
            rightBlock->movementDir = currentDir;
            processed[y][x + 1] = true;
            processed[y][x] = true;
            continue;
          }
        }
      }
    }
  }
}
