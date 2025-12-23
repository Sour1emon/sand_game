#pragma once

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

#define AIR_BLOCK ((Block){.type = AIR, .color = {0, 0, 0, 0}, .movementDir = DIR_NONE})
