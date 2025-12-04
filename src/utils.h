#include "raylib.h"

float fclampf(float value, float min, float max);

void RGBtoHSL(Color rgb, float *h, float *s, float *l);

float HueToRGB(float p, float q, float t);

Color HSLtoRGB(float h, float s, float l);
