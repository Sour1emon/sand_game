#include "block.h"

bool HasGravity(enum BlockType type) {
  switch (type) {
  case SAND:
  case ROCK:
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
