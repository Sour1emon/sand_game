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

// (-5% to +5%)
const int LIGHTNESS_VARIATION = 4;

// (-3% to +3%)
const int SATURATION_VARIATION = 2;

Color GenBlockColor(enum BlockType type) {
  Color base = BLOCK_COLORS[type];
  float h, s, l;

  RGBtoHSL(base, &h, &s, &l);

  if (LIGHTNESS_VARIATION != 0) {
    // Add subtle lightness variation
    float lightness_var =
        ((float)((int)(pcg32() % LIGHTNESS_VARIATION * 2 + 1) -
                 LIGHTNESS_VARIATION)) /
        100.0f;
    l = fclampf(l + lightness_var, 0.0f, 1.0f);
  }

  if (SATURATION_VARIATION != 0) {
    // Add subtle saturation variation
    float sat_var = ((float)((int)(pcg32() % SATURATION_VARIATION * 2 + 1) -
                             SATURATION_VARIATION)) /
                    100.0f;
    s = fclampf(s + sat_var, 0.0f, 1.0f);
  }

  return HSLtoRGB(h, s, l);
}
