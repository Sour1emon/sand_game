#include "block.h"
#include "raylib.h"

#include "rng.h"
#include "utils.h"
#include <stdio.h>

bool HasGravity(enum BlockType type) {
  return HAS_PROPERTY(BLOCKS[type].props, HAS_GRAVITY);
}

bool IsPassible(enum BlockType type) {
  return HAS_PROPERTY(BLOCKS[type].props, IS_PASSIBLE);
}

bool CanSlide(enum BlockType type) {
  return HAS_PROPERTY(BLOCKS[type].props, CAN_SLIDE);
}

bool IsFluid(enum BlockType type) {
  return HAS_PROPERTY(BLOCKS[type].props, IS_FLUID);
}

bool IsGas(enum BlockType type) {
  return HAS_PROPERTY(BLOCKS[type].props, IS_GAS);
}

Color GenBlockColor(enum BlockType type) {
  BlockDef b = BLOCKS[type];
  Color base = b.color;
  float h, s, l;

  RGBtoHSL(base, &h, &s, &l);

  if (b.lightnessVar != 0) {
    // Add subtle lightness variation
    float lightness_var =
        ((float)((int)(pcg32() % b.lightnessVar * 2 + 1) - b.lightnessVar)) /
        100.0f;
    l = fclampf(l + lightness_var, 0.0f, 1.0f);
  }

  if (b.saturationVar != 0) {
    // Add subtle saturation variation
    float sat_var =
        ((float)((int)(pcg32() % b.saturationVar * 2 + 1) - b.saturationVar)) /
        100.0f;
    s = fclampf(s + sat_var, 0.0f, 1.0f);
  }

  return HSLtoRGB(h, s, l);
}
