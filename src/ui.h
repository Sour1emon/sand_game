#pragma once

#include "raylib.h"
#include "state.h"

extern Font font;

bool initFont();

void drawInterface(game_state *state);
