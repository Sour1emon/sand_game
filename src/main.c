#include <raylib.h>
#include <stdio.h>

#include "block.h"

const int PHYSICS_FPS = 20;
const int RENDER_FPS = 60;

const int WORLD_WIDTH = 20;
const int WORLD_HEIGHT = 20;
const int PX_SCALE = 30;

const int SCREEN_WIDTH = WORLD_WIDTH * PX_SCALE;
const int SCREEN_HEIGHT = WORLD_HEIGHT * PX_SCALE;

#define GRID_LINE_COLOR ((Color){50, 50, 50, 255})

#define AIR_BLOCK ((Block){.type = AIR})

void initWorldState(Block world[WORLD_HEIGHT][WORLD_WIDTH]) {
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      world[y][x] = AIR_BLOCK;
    }
  }
}

int main() {

  // Initialize world
  Block world[WORLD_HEIGHT][WORLD_WIDTH];
  initWorldState(world);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sand Game");

  SetTargetFPS(RENDER_FPS);

  float timeSincePhysicsFrame = 0;

  // Main loop
  while (!WindowShouldClose()) {

    float delta = GetFrameTime();
    timeSincePhysicsFrame += delta;

    int mouseX = GetMouseX();
    int mouseY = GetMouseY();

    // The 0.98 is to give it a buffer, hopefully keeping the actual physics fps
    // closer to the target
    if (timeSincePhysicsFrame >= (1.0 / PHYSICS_FPS) * 0.98) {
      timeSincePhysicsFrame = 0.0;

      // Update the world
      for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
          Block block = world[y][x];

          if (y > 0) {

            // Falling
            if (HasGravity(block.type)) {
              if (IsPassible(world[y - 1][x].type)) {
                Block below = world[y - 1][x];
                world[y - 1][x] = block;
                world[y][x] = below;
                continue;
              }
            }

            // Sliding
            if (CanSlide(block.type)) {
              bool isLeftPassible =
                  x > 0 ? IsPassible(world[y - 1][x - 1].type) : false;
              bool isRightPassible = x < WORLD_WIDTH - 1
                                         ? IsPassible(world[y - 1][x + 1].type)
                                         : false;
              if (isLeftPassible && isRightPassible) {
                bool slideLeft = false; // TODO: Make this random
                if (slideLeft) {
                  Block leftBlock = world[y - 1][x - 1];
                  world[y - 1][x - 1] = block;
                  world[y][x] = leftBlock;
                } else {
                  Block rightBlock = world[y - 1][x + 1];
                  world[y - 1][x + 1] = block;
                  world[y][x] = rightBlock;
                }
                continue;
              } else if (isLeftPassible) {
                Block leftBlock = world[y - 1][x - 1];
                world[y - 1][x - 1] = block;
                world[y][x] = leftBlock;
                continue;

              } else if (isRightPassible) {
                Block rightBlock = world[y - 1][x + 1];
                world[y - 1][x + 1] = block;
                world[y][x] = rightBlock;
                continue;
              }
            }
          }
        }
      }
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if (mouseX >= 0 && mouseY >= 0 && mouseX < SCREEN_WIDTH &&
        mouseY < SCREEN_HEIGHT) {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        int gridX = mouseX / PX_SCALE;
        // Correct grid position because the the y coordinates of blocks are
        // flipped before rendering
        int gridY = WORLD_HEIGHT - mouseY / PX_SCALE - 1;
        if (gridX >= 0 && gridX < WORLD_WIDTH && gridY >= 0 &&
            gridY < WORLD_HEIGHT) {
          world[gridY][gridX] = (Block){.type = SAND};
        }
      }
    }

    for (int y = 0; y < WORLD_HEIGHT; y++) {
      int screenY = (WORLD_HEIGHT - y - 1) * PX_SCALE;
      for (int x = 0; x < WORLD_WIDTH; x++) {

        int screenX = x * PX_SCALE;

        Block block = world[y][x];
        switch (block.type) {
        case AIR:
          break;
        case SAND:
          DrawRectangle(screenX, screenY, PX_SCALE, PX_SCALE,
                        (Color){194, 178, 128, 255});
          break;
        case ROCK:
          DrawRectangle(screenX, screenY, PX_SCALE, PX_SCALE,
                        (Color){64, 64, 64, 255});
          break;
        }
      }
    }

    for (int y = 0; y < WORLD_HEIGHT; y++) {
      DrawLine(0, y * PX_SCALE, SCREEN_WIDTH, y * PX_SCALE, GRID_LINE_COLOR);
    }

    for (int x = 0; x < WORLD_HEIGHT; x++) {
      DrawLine(x * PX_SCALE, 0, x * PX_SCALE, SCREEN_HEIGHT, GRID_LINE_COLOR);
    }

    // TODO: Fix the weird mouse bug where moving the mouse past the left window
    // border will blink a box near the right window border for a brief moment

    // Render cursor outline on screen
    if (mouseX >= 0 && mouseY >= 0 && mouseX < SCREEN_WIDTH &&
        mouseY < SCREEN_HEIGHT) {
      int screenX = ((int)mouseX / PX_SCALE) * PX_SCALE;
      int screenY = ((int)mouseY / PX_SCALE) * PX_SCALE;

      DrawRectangleLines(screenX, screenY, PX_SCALE, PX_SCALE, RAYWHITE);
    }

    EndDrawing();
  }

  CloseWindow(); // Close window and clean up

  return 0;
}
