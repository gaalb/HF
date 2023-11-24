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

void handle_space(char* input, char* target, SDL_Event event);

void handle_textinput(char* input, char* composition, SDL_Event event);

void randomize_bots(GameData* game_data);

void init_bots(Bot bots[]);

bool in_rect(SDL_Rect rect, int x, int y);

void go_to_single_game(GameData* game_data);

void go_to_bot_game(GameData* game_data);

void go_to_multi_game(GameData* game_data);

void go_to_menu(GameData* game_data);

void get_name(GameData* game_data);

void reset_leaderboard(GameData* game_data);

void bot1_up(GameData* game_data);

void bot2_up(GameData* game_data);

void bot3_up(GameData* game_data);

void bot4_up(GameData* game_data);

void bot1_down(GameData* game_data);

void bot2_down(GameData* game_data);

void bot3_down(GameData* game_data);

void bot4_down(GameData* game_data);

void go_to_settings(GameData* game_data);

Stats calculate_stats(int len, bool* correct, double* times);

Uint32 idozit(Uint32 ms, void* param);

void run_game(GameData* game_data, Text text, SDL_Rect* word_rects, int btn_W, int btn_H, int btn_top, int input_top, SDL_Rect countdown_box, int kocsi_margo, int dx, int car_right);

void run_single_game(GameData* game_data);

void run_bot_game(GameData* game_data);

void wait_for_keypress();

int compare_wpm(const void *a, const void *b);

void run_multi_game(GameData* game_data);

void main_menu(GameData* game_data);

void settings(GameData* game_data);

void multi_statistics(GameData* game_data);

void statistics(GameData* game_data);

void ask_name(GameData* game_data);

#endif // ENGINE_H
