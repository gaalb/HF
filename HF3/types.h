#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "debugmalloc.h"
#include "constants.h"

typedef struct Bot {
    double expected_wpm;
    int spread_ms;
    int ms;
    char name[HOSSZ];
} Bot;

typedef enum {
    TopLeft = 0,
    Middle = 1
} Position;

typedef struct LeaderboardEntry {
    double wpm;
    char name[HOSSZ];
} LeaderboardEntry;

typedef struct LeaderBoard {
    LeaderboardEntry entries[LEADERBOARD_SIZE];
    int num;
} LeaderBoard;

//sztringek dinamikus tömbje
typedef struct Text {
    int word_count;
    char** words;
} Text;

//text-ek dinamikus tömbje
typedef struct TextArray {
    int text_count;
    Text* texts;
} TextArray;

typedef enum {
    MainMenu = 0,
    SingleGame = 1,
    Settings = 2,
    Statistics = 3,
    AskName = 4
} GameView;

typedef struct Stats {
    double wpm;
    double accuracy;
} Stats;

typedef struct GameData {
    Stats stats;
    LeaderBoard leaderboard;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* underlined;
    TTF_Font* title;
    TextArray* p_textarray;
    GameView game_view;
    const int magas;
    const int szeles;
    const int margo;
    const int kocsi_w;
    const int kocsi_h;
    Bot* bots;
} GameData;

typedef struct Button {
    SDL_Rect rect;
    SDL_Color color;
    SDL_Color str_color;
    char* str;
    void (*func)(GameData*);
} Button;


#endif // TYPES_H
