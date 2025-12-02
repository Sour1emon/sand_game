#include <stdbool.h>
enum BlockType { AIR, SAND, ROCK };

bool HasGravity(enum BlockType type);
bool IsPassible(enum BlockType type);
bool CanSlide(enum BlockType type);

typedef struct {
  enum BlockType type;
} Block;
