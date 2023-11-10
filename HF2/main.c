#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "debugmalloc.h"
#include "text.h"

#define SZELES 1400
#define MAGAS 800
#define FONT "LiberationSerif-Regular.ttf"
#define NEXT_WORD_EVENT SDL_USEREVENT+1

