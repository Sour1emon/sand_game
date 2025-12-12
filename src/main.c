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

const float MIDDLE_BLOCK_SCALE = 1.4;

static inline int wrapBlockTypeIndex(int index) {
  return (index % BLOCK_TYPES_COUNT + BLOCK_TYPES_COUNT) % BLOCK_TYPES_COUNT;
}

static inline int calculateBlockPosX(int i, int startX, float blockSize,
                                     float padding, float scaleAdded) {
  int posX =
      startX + blockSize * TWO_THIRDS + i * blockSize + (i + 1) * padding;

  // For boxes after the middle one, add extra space for the enlarged middle
  // block.
  if (i > 2) {
    posX += blockSize * scaleAdded + padding * scaleAdded;
  }
  return posX;
}

void drawBlockSelectionTriangles(int startX, int startY, int yOffset, int endX,
                                 float blockSize) {
  // Left triangle
  DrawTriangle(
      (Vector2){startX + blockSize * TWO_THIRDS, startY + yOffset},
      (Vector2){startX, startY + blockSize / 2 + yOffset},
      (Vector2){startX + blockSize * TWO_THIRDS, startY + blockSize + yOffset},
      RAYWHITE);

  // Right triangle
  DrawTriangle((Vector2){endX, startY + yOffset},
               (Vector2){endX, startY + blockSize + yOffset},
               (Vector2){endX + blockSize * TWO_THIRDS,
                         startY + blockSize / 2 + yOffset},
               RAYWHITE);
}

void drawBlockAtIndex(int i, int posX, int startY, float blockSize,
                      float scaleAdded, float fontSize) {
  int posY = startY;
  int size = blockSize;

  // This means that the block being drawn is the middle block, which needs to
  // be bigger than all of the other ones
  if (i == 2) {
    size *= MIDDLE_BLOCK_SCALE;
    const char *displayName = BLOCKS[selectedBlockType].displayName;
    Vector2 textSize = MeasureTextEx(font, displayName, fontSize, 2);
    DrawTextEx(font, displayName,
               (Vector2){posX + (size - textSize.x) / 2, startY - textSize.y},
               fontSize, 2, RAYWHITE);
  } else {
    posY += blockSize * scaleAdded / 2;
  }

  int blockTypeIndex = wrapBlockTypeIndex(selectedBlockType + (i - 2));
  DrawRectangle(posX, posY, size, size, BLOCKS[blockTypeIndex].color);
  DrawRectangleLinesEx((Rectangle){posX, posY, size, size}, 1, RAYWHITE);
}

void drawBlockPicker(int startX, int startY) {
  const float BLOCK_SIZE = 35;
  const float PADDING = BLOCK_SIZE * 0.2;
  const float SCALE_ADDED = MIDDLE_BLOCK_SCALE - 1.0;
  const float FONT_SIZE = 20.0f * (BLOCK_SIZE / 50.0);
  const float SPACING_FROM_SELECTOR = 10.0f;

  // Calculate layout.
  // The text used for MeasureTextEx does not really matter as long as its upper
  // case. This ensures the position of everything else is offset based on the
  // block name text that is displayed
  const float HEIGHT_OFFSET = MeasureTextEx(font, "ABC123", FONT_SIZE, 2).y;
  int yOffset = SCALE_ADDED * BLOCK_SIZE / 2 + HEIGHT_OFFSET;

  // Draw blocks
  int endX = startX;
  for (int i = 0; i < 5; i++) {
    endX = calculateBlockPosX(i, startX, BLOCK_SIZE, PADDING, SCALE_ADDED);
    drawBlockAtIndex(i, endX, startY + HEIGHT_OFFSET, BLOCK_SIZE, SCALE_ADDED,
                     FONT_SIZE);
  }

  // Calculate final position for right triangle
  endX = calculateBlockPosX(5, startX, BLOCK_SIZE, PADDING, SCALE_ADDED);

  // Draw navigation triangles
  drawBlockSelectionTriangles(startX, startY, yOffset, endX, BLOCK_SIZE);

  // Draw instruction text
  int selectorWidth = endX + BLOCK_SIZE * TWO_THIRDS - startX;
  const char *message = "Press A/D to switch blocks";
  Vector2 size = MeasureTextEx(font, message, FONT_SIZE, 2);
  DrawTextEx(font, message,
             (Vector2){startX + (selectorWidth - size.x) / 2,
                       startY + BLOCK_SIZE * MIDDLE_BLOCK_SCALE + PADDING +
                           SPACING_FROM_SELECTOR},
             FONT_SIZE, 2, YELLOW);
}

void drawInterface() {
  int startX = WORLD_SCREEN_TOP_LEFT_X;
  int startY = WORLD_SCREEN_BOTTOM_RIGHT_Y + WORLD_DISPLAY_PADDING;
  drawBlockPicker(startX, startY);
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
      selectedBlockType = wrapBlockTypeIndex(selectedBlockType - 1);
    } else if (IsKeyPressed(KEY_D)) {
      selectedBlockType = wrapBlockTypeIndex(selectedBlockType + 1);
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
