#pragma once

#include "raylib.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

int wrapBlockTypeIndex(int index);

float fclampf(float value, float min, float max);

void RGBtoHSL(Color rgb, float *h, float *s, float *l);

float HueToRGB(float p, float q, float t);

Color HSLtoRGB(float h, float s, float l);
