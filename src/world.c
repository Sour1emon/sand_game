#include "world.h"
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
      if (y > 0 && HasGravity(block->type)) {
        // Try falling straight down first
        Block *below = getBlock(x, y - 1);
        if (IsPassible(below->type)) {
          swap(block, below);
          processed[y - 1][x] = true;
          processed[y][x] = true;
          continue;
        }

        // If can't fall straight, try sliding diagonally
        if (CanSlide(block->type)) {
          Block *leftBlock = getBlock(x - 1, y - 1);
          bool isLeftPassible =
              leftBlock != NULL ? IsPassible(leftBlock->type) &&
                                      IsPassible(_state.world[y][x - 1].type)
                                : false;
          Block *rightBlock = getBlock(x + 1, y - 1);
          bool isRightPassible =
              rightBlock != NULL ? IsPassible(rightBlock->type) &&
                                       IsPassible(_state.world[y][x + 1].type)
                                 : false;
          if (isLeftPassible && isRightPassible) {
            bool slideLeft = pcg32_bool();
            swap(slideLeft ? leftBlock : rightBlock, block);
            processed[y - 1][x + (slideLeft ? -1 : 1)] = true;
            processed[y][x] = true;
          } else if (isLeftPassible) {
            swap(leftBlock, block);
            processed[y - 1][x - 1] = true;
            processed[y][x] = true;
          } else if (isRightPassible) {
            swap(rightBlock, block);
            processed[y - 1][x + 1] = true;
            processed[y][x] = true;
          }
        }
      }
    }
  }
}
