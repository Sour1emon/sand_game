#include <raylib.h>

#include "block.h"

const int PHYSICS_FPS = 5;
const int RENDER_FPS = 60;

const int WORLD_WIDTH = 20;
const int WORLD_HEIGHT = 20;
const int PX_SCALE = 30;

const int SCREEN_WIDTH = WORLD_WIDTH * PX_SCALE;
const int SCREEN_HEIGHT = WORLD_HEIGHT * PX_SCALE;

void initWorldState(Block world[WORLD_HEIGHT][WORLD_WIDTH]) {
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      world[y][x].blockType = BLOCK_AIR;
    }
  }
}

#define GRID_LINE_COLOR CLITERAL(Color){245, 245, 245, 50}

int main() {

  // Initialize world
  Block world[WORLD_HEIGHT][WORLD_WIDTH];
  initWorldState(world);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello, Raylib");

  SetTargetFPS(RENDER_FPS);

  float timeSincePhysicsFrame = 0;

  // Main loop
  while (!WindowShouldClose()) {

    float delta = GetFrameTime();
    timeSincePhysicsFrame += delta;

    // The 0.98 is to give it a buffer, hopefully keeping the actual physics fps
    // closer to the target
    if (timeSincePhysicsFrame >= (1.0 / PHYSICS_FPS) * 0.98) {
      timeSincePhysicsFrame = 0.0;

      // Update the world
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (int y = 0; y < WORLD_HEIGHT; y++) {
      DrawLine(0, y * PX_SCALE, SCREEN_WIDTH, y * PX_SCALE, GRID_LINE_COLOR);
    }

    for (int x = 0; x < WORLD_HEIGHT; x++) {
      DrawLine(x * PX_SCALE, 0, x * PX_SCALE, SCREEN_HEIGHT, GRID_LINE_COLOR);
    }

    EndDrawing();
  }

  CloseWindow(); // Close window and clean up

  return 0;
}
