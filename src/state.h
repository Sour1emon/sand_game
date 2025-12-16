#pragma once
#include "block.h"

typedef struct {
  int placeWidth;
  enum BlockType selectedBlockType;
} game_state;

game_state _state;

static inline game_state *getState() { return &_state; }

static inline void initGameState() {
  _state = (game_state){.placeWidth = 1, .selectedBlockType = SAND};
}
