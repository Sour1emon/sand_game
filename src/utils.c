#include "utils.h"
#include <math.h>

// Clamp helper function
float fclampf(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

// Helper functions to convert between RGB and HSL
void RGBtoHSL(Color rgb, float *h, float *s, float *l) {
  float r = rgb.r / 255.0f;
  float g = rgb.g / 255.0f;
  float b = rgb.b / 255.0f;

  float max = fmaxf(fmaxf(r, g), b);
  float min = fminf(fminf(r, g), b);
  float delta = max - min;

  *l = (max + min) / 2.0f;

  if (delta == 0) {
    *h = 0;
    *s = 0;
  } else {
    *s = (*l < 0.5f) ? (delta / (max + min)) : (delta / (2.0f - max - min));

    if (max == r) {
      *h = 60.0f * fmodf(((g - b) / delta), 6.0f);
    } else if (max == g) {
      *h = 60.0f * (((b - r) / delta) + 2.0f);
    } else {
      *h = 60.0f * (((r - g) / delta) + 4.0f);
    }

    if (*h < 0)
      *h += 360.0f;
  }
}

float HueToRGB(float p, float q, float t) {
  if (t < 0)
    t += 1;
  if (t > 1)
    t -= 1;
  if (t < 1.0f / 6.0f)
    return p + (q - p) * 6.0f * t;
  if (t < 1.0f / 2.0f)
    return q;
  if (t < 2.0f / 3.0f)
    return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
  return p;
}

Color HSLtoRGB(float h, float s, float l) {
  Color rgb;

  if (s == 0) {
    rgb.r = rgb.g = rgb.b = (unsigned char)(l * 255);
  } else {
    float q = (l < 0.5f) ? (l * (1 + s)) : (l + s - l * s);
    float p = 2 * l - q;
    float hNorm = h / 360.0f;

    float r = HueToRGB(p, q, hNorm + 1.0f / 3.0f);
    float g = HueToRGB(p, q, hNorm);
    float b = HueToRGB(p, q, hNorm - 1.0f / 3.0f);

    rgb.r = (unsigned char)(r * 255);
    rgb.g = (unsigned char)(g * 255);
    rgb.b = (unsigned char)(b * 255);
  }
  rgb.a = 255;

  return rgb;
}
