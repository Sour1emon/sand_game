#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

enum BlockType { AIR, SAND, ROCK, BLOCK_TYPES_COUNT };

#define ARG_INDEX(x) ((uint64_t)(1 << x))

typedef enum {
  HAS_GRAVITY = ARG_INDEX(0),
  IS_PASSIBLE = ARG_INDEX(1),
  CAN_SLIDE = ARG_INDEX(2)
} BLOCK_PROPERTY;

#define NO_PROPS ((uint64_t)(0))

#define HAS_PROPERTY(flags, prop) ((flags) & (prop) != 0)

#define RGBA(r, g, b, a) ((Color){r, g, b, a})

bool HasGravity(enum BlockType type);
bool IsPassible(enum BlockType type);
bool CanSlide(enum BlockType type);

Color GenBlockColor(enum BlockType type);

typedef struct {
  enum BlockType type;
  Color color;
  uint64_t props;
  // Lightness variation in percent
  uint32_t lightness_var;
  // Saturation variation in percent
  uint32_t saturation_var;

} BlockDef;

static const BlockDef BLOCKS[BLOCK_TYPES_COUNT] = {
    [AIR] = (BlockDef){.type = AIR,
                       .color = RGBA(0, 0, 0, 0),
                       .props = IS_PASSIBLE,
                       .lightness_var = 0,
                       .saturation_var = 0},
    [SAND] = (BlockDef){.type = SAND,
                        .color = RGBA(194, 178, 128, 255),
                        .props = HAS_GRAVITY | CAN_SLIDE,
                        .lightness_var = 4,
                        .saturation_var = 2},
    [ROCK] = (BlockDef){.type = ROCK,
                        .color = RGBA(64, 64, 64, 255),
                        .props = NO_PROPS,
                        .lightness_var = 4,
                        .saturation_var = 2}};

typedef struct {
  enum BlockType type;
  Color color;
} Block;
