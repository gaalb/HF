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

/*
Egy kocsi, mint kirajzolandó objektum adatait
tartalmazó struktúra
*/
typedef struct Car {
    int x, y, w, h; //pozíció, méret
    SDL_Color color1; //a karosszéria színe
    SDL_Color color2; //az ablakok színe
    char name[HOSSZ]; //a neve
} Car;

/*
Egy nem játékos által irányított kocsi mögöttes
logikáját tartalmazó objektum
*/
typedef struct Bot {
    double expected_wpm; //az átlagos wpm-je
    int ms; //a wpm-ből számolható periódusidő
    Car car; //a kirajzoláshoz használt kocsi
    SDL_EventType tick; //az event, amire halandi fog a képernyőn
    bool active; //a tény, hogy játékban van-e
} Bot;

typedef enum  Position {
    TopLeft = 0,
    Middle = 1,
    Right = 2
} Position;

/*
Egy ranglista elem adatai
*/
typedef struct LeaderboardEntry {
    double wpm; //a sebesség
    char name[HOSSZ]; //a neve a játékosnak
} LeaderboardEntry;

/*
Ranglista elemek tömbje: nem kell dinamikus tömb
legyen, mert előre meghatározott, hogy hány ember
lehet a ranglistán (LEADERBOARD_SIZE). Viszont lehet,
hogy ennél kevesebb ember van rajta, ezért tartalmaznia
kell annak tényét, hányan vannak a ranglistán
*/
typedef struct LeaderBoard {
    LeaderboardEntry entries[LEADERBOARD_SIZE];
    int num;
} LeaderBoard;

//sztringek dinamikus tömbje (a stringek is dinamikusan foglaltak)
typedef struct Text {
    int word_count;
    char** words;
} Text;

//text-ek dinamikus tömbje
typedef struct TextArray {
    int text_count;
    Text* texts;
} TextArray;

/*a különböző lehetséges játéknézetek*/
typedef enum GameView {
    MainMenu = 0,
    SingleGame = 1,
    BotGame = 2,
    MultiGame = 3,
    Settings = 4,
    Statistics = 5,
    MultiStatistics = 6,
    AskName = 7
} GameView;

/*
Egy játszma után a készült statisztikák
*/
typedef struct Stats {
    double wpm; //sebesség (szó/perc)
    double accuracy;
} Stats;

/*
Ez az adatstruktúra (amelyből logikusan csak egy lehet) tárolja
az összes adatot, amihez több játéknézetnek is hozzá kell férnie.
Ezen keresztül tudnak kommunikálni egymással a játéknézetek.
*/
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
    Bot* bots;
    Bot* multis;
    int players;
    Car player_car;
} GameData;

/*
Egy gomb adatstruktúrája. Egy gombot leír a színe,
a helye/mérete, a felirata, és a funkciója
*/
typedef struct Button {
    SDL_Rect rect; //elhelyezés + méret
    SDL_Color color; //szín
    SDL_Color str_color; //betűszín
    char* str; //felirat
    void (*func)(GameData*); //a függvényére mutató pointer: ha rákattintunk, ez fusson le
} Button;


#endif // TYPES_H
