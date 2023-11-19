#ifndef CUSTOM_RENDERS_H
#define CUSTOM_RENDERS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include "custom_renders.h"
#include "constants.h"
#include "text.h"

typedef enum {
    TopLeft = 0,
    Middle = 1
} Position;

char* add_space(char* s1, char* s2);

int match_len(char* target, char* input);

bool input_correct(char* target, char* input);

void render_input(char *input, SDL_Rect teglalap, SDL_Color input_color, TTF_Font *font, SDL_Renderer *renderer, char* composition, char* textandcomposition);

void render_string_to_rect_blended(char* str, SDL_Color color, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer);

void render_string_to_rect_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer);

SDL_Rect render_string_blended(char* str, SDL_Color color, TTF_Font* font, int x, int y, SDL_Renderer* renderer, Position position);

SDL_Rect render_string_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, int x, int y, SDL_Renderer* renderer, Position position);

int get_cursor_index(Text text, int target_index, int input_len, int* cursor_len);

SDL_Rect* calculate_Rects(Text text, TTF_Font* font, int x, int y, int w) ;

void clear_screen(SDL_Renderer* renderer, SDL_Color color);

void render_cursor(Text text, SDL_Rect* word_rects, TTF_Font* font, int cursor_index, int cusor_len, SDL_Renderer* renderer);

void render_Text(Text text, TTF_Font* font, TTF_Font* underlined, SDL_Rect* word_rects, SDL_Renderer* renderer, SDL_Color color1, SDL_Color color2, SDL_Color color3, int target_index, char* input);

void render_car(SDL_Renderer* renderer, SDL_Color color1, SDL_Color color2, int x, int y, int w, int h);

#endif // CUSTOM_RENDERS_H
