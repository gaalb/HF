#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "debugmalloc.h"
#include "text.h"
#include "custom_renders.h"
#include "constants.h"
#include "leaderboard.h"
#include "types.h"


bool input_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, TTF_Font *font, SDL_Renderer *renderer, bool* escape);

void handle_backspace(char* input);

void handle_space(char* input, char* target, char* composition, SDL_Event event);

void handle_textinput(char* input, char* composition, SDL_Event event);

#endif // ENGINE_H
