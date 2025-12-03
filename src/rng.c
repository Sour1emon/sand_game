#include "rng.h"

uint32_t rotr32(uint32_t x, unsigned r) { return x >> r | x << (-r & 31); }

uint32_t pcg32(void) {
  uint64_t x = state;
  unsigned count = (unsigned)(x >> 59); // 59 = 64 - 5

  state = x * multiplier + increment;
  x ^= x >> 18;                              // 18 = (64 - 27)/2
  return rotr32((uint32_t)(x >> 27), count); // 27 = 32 - 5}
}

bool pcg32_bool(void) { return pcg32() & 1; }

void pcg32_init(uint64_t seed) {
  state = seed + increment;
  (void)pcg32();
}
