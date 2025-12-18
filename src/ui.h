#pragma once

#include "raylib.h"
#include "state.h"

extern Font font;
extern Font font_bold;

bool initFont();

void drawInterface(game_state *state);
