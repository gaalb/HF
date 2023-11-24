#ifndef TEXT_H
#define TEXT_H
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "debugmalloc.h"
#include "types.h"

void print_text(Text text);

void free_text(Text* text);

void free_textarray(TextArray* textarray);

char* recursive_read(int n, FILE* file);

char* read_word_from_file(FILE* file);

Text read_text_from_file(FILE* file);

TextArray parse_file(char filename[]);

#endif // TEXT_H
