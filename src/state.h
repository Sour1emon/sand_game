#pragma once
#include "block.h"
#include "consts.h"

typedef struct {
  int placeWidth;
  enum BlockType selectedBlockType;
  Block world[WORLD_HEIGHT][WORLD_WIDTH];
} game_state;

extern game_state _state;

void initGameState();
