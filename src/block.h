#include "raylib.h"
#include <stdbool.h>

enum BlockType { AIR, SAND, ROCK, BLOCK_TYPES_COUNT };

bool HasGravity(enum BlockType type);
bool IsPassible(enum BlockType type);
bool CanSlide(enum BlockType type);

Color GenBlockColor(enum BlockType type);

static const Color BLOCK_COLORS[BLOCK_TYPES_COUNT] = {
    [AIR] = (Color){0, 0, 0, 0},
    [SAND] = (Color){194, 178, 128, 255},
    [ROCK] = (Color){64, 64, 64, 255}};

typedef struct {
  enum BlockType type;
  Color color;
} Block;
