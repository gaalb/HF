#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "constants.h"
#include "types.h"

bool load_leaderboard(LeaderBoard* leaderboard);

bool save_leaderboard(LeaderBoard leaderboard);

void add_entry(LeaderBoard* leaderboard, LeaderboardEntry entry);

void remove_entry(LeaderBoard* leaderboard, int idx);

int find_index(LeaderBoard leaderboard, LeaderboardEntry entry);

void print_leaderboard(LeaderBoard leaderboard);

void update_leaderboard(LeaderBoard* leaderboard, LeaderboardEntry entry);

bool top10(LeaderBoard leaderboard, double wpm);

#endif // LEADERBOARD_H

