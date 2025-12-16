#pragma once

#include "raylib.h"

#define min(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b ? _a : _b;                                                         \
  })

#define max(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

int wrapBlockTypeIndex(int index);

float fclampf(float value, float min, float max);

void RGBtoHSL(Color rgb, float *h, float *s, float *l);

float HueToRGB(float p, float q, float t);

Color HSLtoRGB(float h, float s, float l);
