#include "block.h"
#include "raylib.h"

#include "rng.h"
#include "utils.h"

bool HasGravity(enum BlockType type) {
  switch (type) {
  case SAND:
    return true;
  default:
    return false;
  }
}

bool IsPassible(enum BlockType type) {
  switch (type) {
  case AIR:
    return true;
  default:
    return false;
  }
}

bool CanSlide(enum BlockType type) {
  switch (type) {
  case SAND:
    return true;
  default:
    return false;
  }
}

Color GenBlockColor(enum BlockType type) {
  BlockDef b = BLOCKS[type];
  Color base = b.color;
  float h, s, l;

  RGBtoHSL(base, &h, &s, &l);

  if (b.lightness_var != 0) {
    // Add subtle lightness variation
    float lightness_var =
        ((float)((int)(pcg32() % b.lightness_var * 2 + 1) - b.lightness_var)) /
        100.0f;
    l = fclampf(l + lightness_var, 0.0f, 1.0f);
  }

  if (b.saturation_var != 0) {
    // Add subtle saturation variation
    float sat_var = ((float)((int)(pcg32() % b.saturation_var * 2 + 1) -
                             b.saturation_var)) /
                    100.0f;
    s = fclampf(s + sat_var, 0.0f, 1.0f);
  }

  return HSLtoRGB(h, s, l);
}
