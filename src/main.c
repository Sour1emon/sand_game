#include <err.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "block.h"
#include "rng.h"

#define const_min(a, b) ((a) < (b) ? (a) : (b))
#define const_max(a, b) ((a) > (b) ? (a) : (b))

#define TWO_THIRDS (2.0f / 3.0f)

// Better than using define to make these actually constant
enum {
  PHYSICS_FPS = 20,
  RENDER_FPS = 60,

  WORLD_WIDTH = 60,
  WORLD_HEIGHT = 60,
  PX_SCALE = 10,

  WORLD_DISPLAY_PADDING = 20,

  INTERFACE_HEIGHT = 200,
  INTERFACE_WIDTH = 600,

  SCREEN_WIDTH = const_max(WORLD_WIDTH * PX_SCALE + WORLD_DISPLAY_PADDING * 2,
                           INTERFACE_WIDTH),
  SCREEN_HEIGHT =
      WORLD_HEIGHT * PX_SCALE + WORLD_DISPLAY_PADDING * 2 + INTERFACE_HEIGHT,

  WORLD_SCREEN_TOP_LEFT_X = (SCREEN_WIDTH - WORLD_WIDTH * PX_SCALE) / 2,
  WORLD_SCREEN_TOP_LEFT_Y = WORLD_DISPLAY_PADDING,
  WORLD_SCREEN_BOTTOM_RIGHT_X = (SCREEN_WIDTH + WORLD_WIDTH * PX_SCALE) / 2,
  WORLD_SCREEN_BOTTOM_RIGHT_Y =
      (WORLD_HEIGHT * PX_SCALE + WORLD_DISPLAY_PADDING)
};

#define GRID_LINE_COLOR ((Color){50, 50, 50, 255})

#define AIR_BLOCK ((Block){.type = AIR, .color = {0, 0, 0, 0}})

void initWorldState(Block world[WORLD_HEIGHT][WORLD_WIDTH]) {
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      world[y][x] = AIR_BLOCK;
    }
  }
}

Block world[WORLD_HEIGHT][WORLD_WIDTH];

Font font;

enum BlockType selectedBlockType = SAND;

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

// TODO: Clean up this function because currently it is a mess
void drawInterface() {
  int startX = WORLD_SCREEN_TOP_LEFT_X;
  int startY = WORLD_SCREEN_BOTTOM_RIGHT_Y + WORLD_DISPLAY_PADDING;

  // Block picker
  {
    const float BLOCK_SELECTION_SIZE = 50;
    const float BLOCK_SELECTION_PADDING = 10;
    const float MIDDLE_BLOCK_SCALE = 1.4;
    const float SCALE_ADDED = (MIDDLE_BLOCK_SCALE - 1.0);

    const float FONT_SIZE = 20.0f;
    const float HEIGHT_OFFSET = MeasureTextEx(font, "ABC123", FONT_SIZE, 2).y;

    int yOffset = SCALE_ADDED * BLOCK_SELECTION_SIZE / 2 + HEIGHT_OFFSET;
    DrawTriangle(
        (Vector2){startX + BLOCK_SELECTION_SIZE * TWO_THIRDS, startY + yOffset},
        (Vector2){startX, startY + BLOCK_SELECTION_SIZE / 2 + yOffset},
        (Vector2){startX + BLOCK_SELECTION_SIZE * TWO_THIRDS,
                  startY + BLOCK_SELECTION_SIZE + yOffset},
        RAYWHITE);

    Vector2 charSize = MeasureTextEx(font, "A", FONT_SIZE, 0);

    DrawText("A", startX + BLOCK_SELECTION_SIZE / 2 - charSize.x / 2.0 - 2.0,
             startY + BLOCK_SELECTION_SIZE / 2 + yOffset - charSize.y / 2, 20,
             RED);

    int posX;

    // The 6th time around is to ensure that posX is in the correct place for
    // the other triangle to go
    for (int i = 0; i < 6; i++) {
      posX = startX + BLOCK_SELECTION_SIZE * TWO_THIRDS +
             i * BLOCK_SELECTION_SIZE + (i + 1) * BLOCK_SELECTION_PADDING +
             (i > 2 ? (BLOCK_SELECTION_SIZE + BLOCK_SELECTION_PADDING) *
                          SCALE_ADDED * 2
                    : 0);
      if (i == 5) {
        break;
      }
      int posY = startY + HEIGHT_OFFSET;
      int size = BLOCK_SELECTION_SIZE;
      if (i == 2) {
        size *= MIDDLE_BLOCK_SCALE;
        posX += BLOCK_SELECTION_PADDING * SCALE_ADDED +
                BLOCK_SELECTION_SIZE * SCALE_ADDED / 2;

      } else {
        posY += BLOCK_SELECTION_SIZE * SCALE_ADDED / 2;
      }
      DrawRectangleLines(posX, posY, size, size, RAYWHITE);
      const float INNER_BLOCK_SCALE = 0.8;
      const float innerBlockSize = size * INNER_BLOCK_SCALE;
      const float offset = ((1.0 - INNER_BLOCK_SCALE) * size) / 2;

      // Wrap around block type
      int blockTypeIndex = ((int)selectedBlockType) + (i - 2);
      blockTypeIndex =
          (blockTypeIndex % BLOCK_TYPES_COUNT + BLOCK_TYPES_COUNT) %
          BLOCK_TYPES_COUNT;
      DrawRectangle(posX + offset, posY + offset, innerBlockSize,
                    innerBlockSize,
                    BLOCKS[((enum BlockType)blockTypeIndex)].color);
    }

    DrawTriangle((Vector2){posX, startY + yOffset},
                 (Vector2){posX, startY + BLOCK_SELECTION_SIZE + yOffset},
                 (Vector2){posX + BLOCK_SELECTION_SIZE * TWO_THIRDS,
                           startY + BLOCK_SELECTION_SIZE / 2 + yOffset},
                 RAYWHITE);

    charSize = MeasureTextEx(font, "D", FONT_SIZE, 0);

    // Most of these values are here to make it look nice (they were randomly
    // selected after trial and error)
    DrawText("D", posX + BLOCK_SELECTION_SIZE / 4 - charSize.x / 2.0,
             startY + BLOCK_SELECTION_SIZE / 2 + yOffset - charSize.y / 2, 20,
             RED);
  }
}

int main() {

  pcg32_init((uint64_t)time(NULL));
  // Initialize world
  initWorldState(world);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sand Game");

  font = LoadFontEx("resources/pixantiqua.ttf", 32, 0, 250);

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
        if (gridX >= 0 && gridX < WORLD_WIDTH && gridY >= 0 &&
            gridY < WORLD_HEIGHT) {

          // Make sure the color does not change randomly
          if (world[gridY][gridX].type != selectedBlockType) {
            world[gridY][gridX] =
                (Block){.type = selectedBlockType,
                        .color = GenBlockColor(selectedBlockType)};
          }
        }
      }
    }

    if (IsKeyPressed(KEY_A)) {
      if (selectedBlockType == 0) {
        selectedBlockType = BLOCK_TYPES_COUNT - 1;
      } else {
        selectedBlockType -= 1;
      }
    } else if (IsKeyPressed(KEY_D)) {
      selectedBlockType += 1;
      if (selectedBlockType == BLOCK_TYPES_COUNT) {
        selectedBlockType = 0;
      }
    }

    for (int y = 0; y < WORLD_HEIGHT; y++) {
      int screenY = (WORLD_HEIGHT - y - 1) * PX_SCALE + WORLD_SCREEN_TOP_LEFT_Y;
      for (int x = 0; x < WORLD_WIDTH; x++) {

        int screenX = x * PX_SCALE + WORLD_SCREEN_TOP_LEFT_X;

        Block block = world[y][x];
        switch (block.type) {
        case AIR:
          break;
        case SAND:
        case ROCK:
          DrawRectangle(screenX, screenY, PX_SCALE, PX_SCALE, block.color);
          break;
        case BLOCK_TYPES_COUNT:
          // This branch should never be reached
          errx(EXIT_FAILURE, "Found block of type BLOCK_TYPES_COUNT which "
                             "should never happen");
        }
      }
    }

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
