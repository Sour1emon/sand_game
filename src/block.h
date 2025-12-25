#pragma once

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

enum BlockType { AIR, SAND, GRAVEL, ROCK, WATER, SMOKE, BLOCK_TYPES_COUNT };

#define ARG_INDEX(x) ((uint64_t)(1 << x))

typedef enum {
  HAS_GRAVITY = ARG_INDEX(0),
  IS_PASSIBLE = ARG_INDEX(1),
  CAN_SLIDE = ARG_INDEX(2),
  IS_FLUID = ARG_INDEX(3),
  IS_GAS = ARG_INDEX(4)
} BLOCK_PROPERTY;

#define NO_PROPS ((uint64_t)(0))

#define HAS_PROPERTY(flags, prop) (((flags) & (prop)) != 0)

#define RGBA(r, g, b, a) ((Color){r, g, b, a})

Color GenBlockColor(enum BlockType type);

typedef enum { DIR_NONE, DIR_LEFT, DIR_RIGHT } Direction;

typedef struct {
  enum BlockType type;
  const char *displayName;
  Color color;
  uint64_t props;
  // Lightness variation in percent
  int lightnessVar;
  // Saturation variation in percent
  int saturationVar;
} BlockDef;

static const BlockDef BLOCKS[BLOCK_TYPES_COUNT] = {
    [AIR] = (BlockDef){.type = AIR,
                       .displayName = "Air",
                       .color = RGBA(0, 0, 0, 0),
                       .props = IS_PASSIBLE,
                       .lightnessVar = 0,
                       .saturationVar = 0},
    [SAND] = (BlockDef){.type = SAND,
                        .displayName = "Sand",
                        .color = RGBA(194, 178, 128, 255),
                        .props = HAS_GRAVITY | CAN_SLIDE,
                        .lightnessVar = 4,
                        .saturationVar = 2},
    [GRAVEL] = (BlockDef){.type = GRAVEL,
                          .displayName = "Gravel",
                          .color = RGBA(114, 114, 114, 255),
                          .props = HAS_GRAVITY | CAN_SLIDE,
                          .lightnessVar = 4,
                          .saturationVar = 2},
    [ROCK] = (BlockDef){.type = ROCK,
                        .displayName = "Rock",
                        .color = RGBA(171, 171, 171, 255),
                        .props = NO_PROPS,
                        .lightnessVar = 4,
                        .saturationVar = 2},
    [WATER] = (BlockDef){.type = WATER,
                         .displayName = "Water",
                         .color = RGBA(28, 163, 236, 255),
                         .props = HAS_GRAVITY | CAN_SLIDE | IS_FLUID,
                         .lightnessVar = 4,
                         .saturationVar = 2},
    [SMOKE] = (BlockDef){.type = SMOKE,
                         .displayName = "Smoke",
                         .color = RGBA(56, 56, 56, 255),
                         .props = IS_PASSIBLE | IS_GAS,
                         .lightnessVar = 4,
                         .saturationVar = 2}};

typedef struct {
  enum BlockType type;
  Color color;
  // Only for fluids so they keep moving in the same direction
  // For non fluids, this will have a garbage value because it will never be
  // initialized, and hopefully never used
  Direction movementDir;
} Block;

bool HasGravity(Block *block);
bool IsPassible(Block *block);
bool CanSlide(Block *block);
bool IsFluid(Block *block);
bool IsGas(Block *block);