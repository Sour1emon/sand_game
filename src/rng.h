#include <stdbool.h>
#include <stdint.h>

// https://en.wikipedia.org/wiki/Permuted_congruential_generator

static uint64_t state = 0x4d595df4d0f33173;
static uint64_t const multiplier = 6364136223846793005u;
static uint64_t const increment = 1442695040888963407u;

uint32_t rotr32(uint32_t x, unsigned r);

uint32_t pcg32(void);
bool pcg32_bool(void);
float pcg32_float(void);

void pcg32_init(uint64_t seed);
