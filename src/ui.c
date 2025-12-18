#include "ui.h"
#include "block.h"
#include "consts.h"
#include "state.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

Font font;
Font font_bold;

// Return false if the font fails to load.
bool initFont() {
  // TODO: Ensure font loads correctly
  font = LoadFontEx("resources/PixelifySans-Regular.ttf", 32, NULL, 250);
  font_bold = LoadFontEx("resources/PixelifySans-Bold.ttf", 32, NULL, 250);

  return true;
}

const float MIDDLE_BLOCK_SCALE = 1.4;

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

static void drawBlockSelectionTriangles(int startX, int startY, int yOffset,
                                        int endX, float blockSize) {
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

static void drawBlockAtIndex(game_state *state, int i, int posX, int startY,
                             float blockSize, float scaleAdded,
                             float fontSize) {
  int posY = startY;
  int size = blockSize;

  // This means that the block being drawn is the middle block, which needs to
  // be bigger than all of the other ones
  if (i == 2) {
    size *= MIDDLE_BLOCK_SCALE;
    const char *displayName = BLOCKS[state->selectedBlockType].displayName;
    Vector2 textSize = MeasureTextEx(font, displayName, fontSize, 0.0);
    DrawTextEx(font, displayName,
               (Vector2){posX + (size - textSize.x) / 2, startY - textSize.y},
               fontSize, 0.02, RAYWHITE);
  } else {
    posY += blockSize * scaleAdded / 2;
  }

  int blockTypeIndex = wrapBlockTypeIndex(state->selectedBlockType + (i - 2));
  DrawRectangle(posX, posY, size, size, BLOCKS[blockTypeIndex].color);
  DrawRectangleLinesEx((Rectangle){posX, posY, size, size}, 1, RAYWHITE);
}

static Vector2 drawBlockPicker(game_state *state, int startX, int startY) {
  const float BLOCK_SIZE = 35;
  const float PADDING = BLOCK_SIZE * 0.2;
  const float SCALE_ADDED = MIDDLE_BLOCK_SCALE - 1.0;
  const float FONT_SIZE = 20.0f * (BLOCK_SIZE / 50.0);
  const float SPACING_FROM_SELECTOR = 10.0f;

  // Calculate layout.
  // The text used for MeasureTextEx does not really matter as long as its upper
  // case. This ensures the position of everything else is offset based on the
  // block name text that is displayed
  const float HEIGHT_OFFSET = MeasureTextEx(font, "ABC123", FONT_SIZE, 0.0).y;
  int yOffset = SCALE_ADDED * BLOCK_SIZE / 2 + HEIGHT_OFFSET;

  // Draw blocks
  int endX = startX;
  for (int i = 0; i < 5; i++) {
    endX = calculateBlockPosX(i, startX, BLOCK_SIZE, PADDING, SCALE_ADDED);
    drawBlockAtIndex(state, i, endX, startY + HEIGHT_OFFSET, BLOCK_SIZE,
                     SCALE_ADDED, FONT_SIZE);
  }

  // Calculate final position for right triangle
  endX = calculateBlockPosX(5, startX, BLOCK_SIZE, PADDING, SCALE_ADDED);

  // Draw navigation triangles
  drawBlockSelectionTriangles(startX, startY, yOffset, endX, BLOCK_SIZE);

  // Draw instruction text
  int selectorWidth = endX + BLOCK_SIZE * TWO_THIRDS - startX;
  const char *message = "Press A/D to switch blocks";
  Vector2 size = MeasureTextEx(font, message, FONT_SIZE, 0.0);
  const float y = startY + BLOCK_SIZE * MIDDLE_BLOCK_SCALE + PADDING +
                  SPACING_FROM_SELECTOR;
  DrawTextEx(font, message, (Vector2){startX + (selectorWidth - size.x) / 2, y},
             FONT_SIZE, 0.0, YELLOW);
  return (Vector2){endX + BLOCK_SIZE * TWO_THIRDS, y + size.y};
}

static void drawBlockPlaceWidth(game_state *state, int startX, int startY) {
  char *str;
  asprintf(&str, "Block Place Width: %d", state->placeWidth);
  DrawTextEx(font, str, (Vector2){startX, startY}, 20.0f, 0.0, RAYWHITE);
}

void drawInterface(game_state *state) {
  int startX = WORLD_SCREEN_TOP_LEFT_X;
  int startY = WORLD_SCREEN_BOTTOM_RIGHT_Y + WORLD_DISPLAY_PADDING;
  Vector2 end = drawBlockPicker(state, startX, startY);
  drawBlockPlaceWidth(state, startX, end.y + 10);
}
