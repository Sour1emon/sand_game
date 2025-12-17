#pragma once
#include "block.h"
#include "consts.h"

typedef struct {
  int placeWidth;
  enum BlockType selectedBlockType;
  Block world[WORLD_HEIGHT][WORLD_WIDTH];
} game_state;

game_state _state;

static inline void initGameState() {
  _state = (game_state){.placeWidth = 1, .selectedBlockType = SAND};
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      _state.world[y][x] = AIR_BLOCK;
    }
  }
}
