#include <err.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "block.h"
#include "consts.h"
#include "keymap.h"
#include "rng.h"
#include "state.h"
#include "ui.h"
#include "utils.h"
#include "world.h"

void ProcessKeys(game_state *state) {
  // Keybinds for switching the selected block type
  if (IsKeyPressed(SELECTED_BLOCK_LEFT)) {
    state->selectedBlockType = wrapBlockTypeIndex(state->selectedBlockType - 1);
  } else if (IsKeyPressed(SELECTED_BLOCK_RIGHT)) {
    state->selectedBlockType = wrapBlockTypeIndex(state->selectedBlockType + 1);
  }

  if (IsKeyPressed(INCREASE_PLACE_WIDTH)) {
    // Make sure placeWidth is always odd
    state->placeWidth =
        min(state->placeWidth + 2,
            WORLD_WIDTH % 2 == 0 ? WORLD_WIDTH + 1 : WORLD_WIDTH);
  } else if (IsKeyPressed(DECREASE_PLACE_WIDTH)) {
    state->placeWidth = max(state->placeWidth - 2, 1);
  }
}

int main() {

  pcg32_init((uint64_t)time(NULL));

  // TODO: Add the world to the game state

  // Initialize world
  initWorldState();

  // Init game state
  initGameState();

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sand Game");

  initFont();

  SetTextLineSpacing(16);

  SetTargetFPS(RENDER_FPS);

  float timeSincePhysicsFrame = 0;

  // Main loop
  while (!WindowShouldClose()) {
    game_state *state = getState();

    ProcessKeys(state);

    float delta = GetFrameTime();
    timeSincePhysicsFrame += delta;

    int mouseX = GetMouseX();
    int mouseY = GetMouseY();

    // The 0.98 is to give it a buffer, hopefully keeping the actual physics
    // fps closer to the target
    if (timeSincePhysicsFrame >= (1.0 / PHYSICS_FPS) * 0.98) {
      timeSincePhysicsFrame = 0.0;

      // Update the world
      worldTick();
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if (mouseX >= WORLD_SCREEN_TOP_LEFT_X &&
        mouseY >= WORLD_SCREEN_TOP_LEFT_Y &&
        mouseX < WORLD_SCREEN_BOTTOM_RIGHT_X &&
        mouseY < WORLD_SCREEN_BOTTOM_RIGHT_Y) {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        int gridX = (mouseX - WORLD_SCREEN_TOP_LEFT_X) / PX_SCALE;
        // Correct grid position because the the y coordinates of blocks are
        // flipped before rendering
        int gridY =
            WORLD_HEIGHT - (mouseY - WORLD_SCREEN_TOP_LEFT_Y) / PX_SCALE - 1;
        int half = state->placeWidth / 2;

        for (int x = max(gridX + -half, 0);
             x <= min(gridX + half, WORLD_WIDTH - 1); x++) {
          for (int y = max(-half + gridY, 0);
               y <= min(half + gridY, WORLD_HEIGHT - 1); y++) {

            Block *targetBlock = getBlock(x, y);
            // Make sure the color does not change randomly
            // We do not need bounds checking because that is already handled in
            // GetBlock
            if (targetBlock != NULL &&
                targetBlock->type != state->selectedBlockType) {
              setBlock(
                  x, y,
                  (Block){.type = state->selectedBlockType,
                          .color = GenBlockColor(state->selectedBlockType)});
            }
          }
        }
      }
    }

    // Draw the blocks on the screen
    for (int y = 0; y < WORLD_HEIGHT; y++) {
      int screenY = (WORLD_HEIGHT - y - 1) * PX_SCALE + WORLD_SCREEN_TOP_LEFT_Y;
      for (int x = 0; x < WORLD_WIDTH; x++) {

        int screenX = x * PX_SCALE + WORLD_SCREEN_TOP_LEFT_X;

        Block *block = getBlock(x, y);
        switch (block->type) {
        case AIR:
          break;
        case SAND:
        case ROCK:
          DrawRectangle(screenX, screenY, PX_SCALE, PX_SCALE, block->color);
          break;
        case BLOCK_TYPES_COUNT:
          // This branch should never be reached
          errx(EXIT_FAILURE, "Found block of type BLOCK_TYPES_COUNT which "
                             "should never happen");
        }
      }
    }

    // Draw grid
    for (int y = 0; y < WORLD_HEIGHT + 1; y++) {
      DrawLine(WORLD_SCREEN_TOP_LEFT_X, y * PX_SCALE + WORLD_SCREEN_TOP_LEFT_Y,
               WORLD_SCREEN_BOTTOM_RIGHT_X,
               y * PX_SCALE + WORLD_SCREEN_TOP_LEFT_Y, GRID_LINE_COLOR);
    }

    for (int x = 0; x < WORLD_WIDTH + 1; x++) {
      DrawLine(x * PX_SCALE + WORLD_SCREEN_TOP_LEFT_X, WORLD_SCREEN_TOP_LEFT_Y,
               x * PX_SCALE + WORLD_SCREEN_TOP_LEFT_X,
               WORLD_SCREEN_BOTTOM_RIGHT_Y, GRID_LINE_COLOR);
    }

    // TODO: Fix the weird mouse bug where moving the mouse past the left
    // window border will blink a box near the right window border for a brief
    // moment

    // Render cursor outline on screen
    if (mouseX >= WORLD_SCREEN_TOP_LEFT_X &&
        mouseY >= WORLD_SCREEN_TOP_LEFT_Y &&
        mouseX < WORLD_SCREEN_BOTTOM_RIGHT_X &&
        mouseY < WORLD_SCREEN_BOTTOM_RIGHT_Y) {
      int screenX = ((int)mouseX / PX_SCALE) * PX_SCALE;
      int screenY = ((int)mouseY / PX_SCALE) * PX_SCALE;

      DrawRectangleLines(screenX - (state->placeWidth - 1) / 2 * PX_SCALE,
                         screenY - (state->placeWidth - 1) / 2 * PX_SCALE,
                         PX_SCALE * state->placeWidth,
                         PX_SCALE * state->placeWidth, RAYWHITE);
    }

    // Draw the interface at the bottom of the screen
    drawInterface(state);

    EndDrawing();
  }

  CloseWindow(); // Close window and clean up

  return 0;
}
