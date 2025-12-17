#include "state.h"

game_state _state;

void initGameState() {
  _state = (game_state){.placeWidth = 1, .selectedBlockType = SAND};
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      _state.world[y][x] = AIR_BLOCK;
    }
  }
}
