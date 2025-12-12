#include "world.h"
#include "consts.h"
#include "rng.h"
#include <stdlib.h>

Block world[WORLD_HEIGHT][WORLD_WIDTH];

enum BlockType selectedBlockType = SAND;

Block *getBlock(unsigned int x, unsigned int y) {
  if (x > WORLD_WIDTH || y > WORLD_HEIGHT) {
    return NULL;
  }
  return &world[y][x];
}

bool setBlock(unsigned int x, unsigned int y, Block block) {
  if (x > WORLD_WIDTH || y > WORLD_HEIGHT) {
    return false;
  }
  world[y][x] = block;
  return true;
}

void initWorldState() {
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      setBlock(x, y, AIR_BLOCK);
    }
  }
}

void worldTick() {

  bool processed[WORLD_HEIGHT][WORLD_WIDTH] = {false};

  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (unsigned int x = 0; x < WORLD_WIDTH; x++) {
      if (processed[y][x]) {
        continue;
      }
      Block block = world[y][x];
      if (y > 0 && HasGravity(block.type)) {

        // Try falling straight down first
        if (IsPassible(world[y - 1][x].type)) {
          Block below = world[y - 1][x];
          world[y - 1][x] = block;
          world[y][x] = below;
          processed[y - 1][x] = true;
          processed[y][x] = true;
          continue;
        }

        // If can't fall straight, try sliding diagonally
        if (CanSlide(block.type)) {
          bool isLeftPassible = x > 0 ? IsPassible(world[y - 1][x - 1].type) &&
                                            IsPassible(world[y][x - 1].type)
                                      : false;
          bool isRightPassible = x < WORLD_WIDTH - 1
                                     ? IsPassible(world[y - 1][x + 1].type) &&
                                           IsPassible(world[y][x + 1].type)
                                     : false;
          if (isLeftPassible && isRightPassible) {
            bool slideLeft = pcg32_bool();
            if (slideLeft) {
              Block leftBlock = world[y - 1][x - 1];
              world[y - 1][x - 1] = block;
              world[y][x] = leftBlock;
            } else {
              Block rightBlock = world[y - 1][x + 1];
              world[y - 1][x + 1] = block;
              world[y][x] = rightBlock;
            }
            processed[y - 1][x + (slideLeft ? -1 : 1)] = true;
            processed[y][x] = true;
          } else if (isLeftPassible) {
            Block leftBlock = world[y - 1][x - 1];
            world[y - 1][x - 1] = block;
            world[y][x] = leftBlock;
            processed[y - 1][x - 1] = true;
            processed[y][x] = true;
          } else if (isRightPassible) {
            Block rightBlock = world[y - 1][x + 1];
            world[y - 1][x + 1] = block;
            world[y][x] = rightBlock;
            processed[y - 1][x + 1] = true;
            processed[y][x] = true;
          }
        }
      }
    }
  }
}
