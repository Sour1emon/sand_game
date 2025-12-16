#pragma once

#include "block.h"

Block *getBlock(unsigned int x, unsigned int y);

bool setBlock(unsigned int x, unsigned int y, Block block);

void initWorldState();

void worldTick();
