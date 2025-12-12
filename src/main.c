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
#include "ui.h"
#include "utils.h"
#include "world.h"

int main() {

  pcg32_init((uint64_t)time(NULL));
  // Initialize world
  initWorldState();

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sand Game");

  initFont();

  SetTextLineSpacing(16);

  SetTargetFPS(RENDER_FPS);

  float timeSincePhysicsFrame = 0;

  // Main loop
  while (!WindowShouldClose()) {

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
        // Make sure the color does not change randomly
        // We do not need bounds checking because that is already handled in
        // GetBlock
        Block *targetBlock = getBlock(gridY, gridX);
        if (targetBlock != NULL && targetBlock->type != selectedBlockType) {
          setBlock(gridX, gridY,
                   (Block){.type = selectedBlockType,
                           .color = GenBlockColor(selectedBlockType)});
        }
      }
    }

    // Keybinds for switching the selected block type
    if (IsKeyPressed(SELECTED_BLOCK_LEFT)) {
      selectedBlockType = wrapBlockTypeIndex(selectedBlockType - 1);
    } else if (IsKeyPressed(SELECTED_BLOCK_RIGHT)) {
      selectedBlockType = wrapBlockTypeIndex(selectedBlockType + 1);
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

      DrawRectangleLines(screenX, screenY, PX_SCALE, PX_SCALE, RAYWHITE);
    }

    // Draw the interface at the bottom of the screen
    drawInterface();

    EndDrawing();
  }

  CloseWindow(); // Close window and clean up

  return 0;
}
