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


/*A programozás alapjai 1 házi feladat: Typeracer
Gaál Botond
CRQEYD
2023.11.12.*/

/*Az osszes SDL-el kapcsolatos dolog inicializálása:
-ablak nyitás
-renderer készítés
-font megnyitása
*/
void sdl_init(int szeles, int magas, const char* tipus, SDL_Window** pwindow, SDL_Renderer** prenderer, TTF_Font** pfont, TTF_Font** punderlined, TTF_Font** ptitle) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow("Typeracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szeles, magas, 0);
    if (window == NULL) {
        SDL_Log("Nem hozhato letre az ablak: %s", SDL_GetError());
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Nem hozhato letre a megjelenito: %s", SDL_GetError());
        exit(1);
    }
    TTF_Init();
    TTF_Font *font = TTF_OpenFont(tipus, 30);
    if (font == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_Font *title = TTF_OpenFont(tipus, 80);
    if (font == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_SetFontStyle(title, TTF_STYLE_BOLD);
    TTF_Font *underlined = TTF_OpenFont(tipus, 30);
    if (underlined == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_SetFontStyle(underlined, TTF_STYLE_UNDERLINE);
    *pwindow = window;
    *prenderer = renderer;
    *pfont = font;
    *ptitle = title;
    *punderlined = underlined;
    #ifdef __WIN32__
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    #endif
}

void sdl_close(SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **pfont, TTF_Font **punderlined, TTF_Font **ptitle) {
    SDL_DestroyRenderer(*prenderer);
    *prenderer = NULL;

    SDL_DestroyWindow(*pwindow);
    *pwindow = NULL;

    TTF_CloseFont(*pfont);
    *pfont = NULL;

    TTF_CloseFont(*punderlined);
    *pfont = NULL;

    TTF_CloseFont(*ptitle);
    *pfont = NULL;

    SDL_Quit();
}

void randomize_bots(GameData* game_data) {
    for (int i=0; i<BOT_NUM; i++) {
        game_data->bots[i].ms = (int)(60000/(game_data->bots[i].expected_wpm+(rand()%10)-5));
        SDL_Color color = {rand()%255, rand()%255, rand()%255};
        game_data->bots[i].car.color1 = color;
        game_data->bots[i].car.x = game_data->margo;
        game_data->bots[i].active = true;
        printf("randomize output: %s ms: %d\n", game_data->bots[i].car.name, game_data->bots[i].ms);
    }
}

void init_bots(Bot bots[]) {
    for (int i=0; i<BOT_NUM; i++) {
        Car car = {0, 0, 0, 0, {255, 255, 255}, {0, 0, 0}, ""};
        sprintf(car.name, "BOT%d", i+1);
        bots[i].expected_wpm = 30 + i*5;
        bots[i].ms = (int)(60000/(bots[i].expected_wpm+(rand()%10)-5));
        bots[i].tick = TICK_SEC+i+1;
        bots[i].car = car;
        bots[i].active = false;
    }
}

bool in_rect(SDL_Rect rect, int x, int y) {
    bool X = (x >= rect.x) && (x <= rect.x + rect.w);
    bool Y = (y >= rect.y) && (y <= rect.y + rect.h);
    return X && Y;
}

void go_to_single_game(GameData* game_data) {
    printf("Clicked SINGLE_GAME\n");
    game_data->game_view = SingleGame;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void go_to_bot_game(GameData* game_data) {
    printf("Clicked BOT_GAME\n");
    game_data->game_view = BotGame;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void go_to_multi_game(GameData* game_data) {
    printf("Clicked BOT_GAME\n");
    game_data->game_view = MultiGame;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}


void go_to_menu(GameData* game_data) {
    printf("Clicked MENU\n");
    game_data->game_view = MainMenu;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void get_name(GameData* game_data) {
    printf("Clicked ASKNAME\n");
    game_data->game_view = AskName;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void reset_leaderboard(GameData* game_data) {
    game_data->leaderboard.num = 0;
    save_leaderboard(game_data->leaderboard);
}

void bot1_up(GameData* game_data) {
    if (game_data->bots[0].expected_wpm < 200)
        game_data->bots[0].expected_wpm += 5;
}

void bot2_up(GameData* game_data) {
    if (game_data->bots[1].expected_wpm < 200)
        game_data->bots[1].expected_wpm += 5;
}

void bot3_up(GameData* game_data) {
    if (game_data->bots[2].expected_wpm < 200)
        game_data->bots[2].expected_wpm += 5;
}

void bot4_up(GameData* game_data) {
    if (game_data->bots[3].expected_wpm < 200)
        game_data->bots[3].expected_wpm += 5;
}

void bot1_down(GameData* game_data) {
    if (game_data->bots[0].expected_wpm > 20)
        game_data->bots[0].expected_wpm -= 5;
}

void bot2_down(GameData* game_data) {
    if (game_data->bots[1].expected_wpm > 20)
        game_data->bots[1].expected_wpm -= 5;
}

void bot3_down(GameData* game_data) {
    if (game_data->bots[2].expected_wpm > 20)
        game_data->bots[2].expected_wpm -= 5;
}

void bot4_down(GameData* game_data) {
    if (game_data->bots[3].expected_wpm > 20)
        game_data->bots[3].expected_wpm -= 5;
}

void go_to_settings(GameData* game_data) {
    printf("Clicked SETTINGS\n");
    game_data->game_view = Settings;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

Stats calculate_stats(int len, bool* correct, double* times) {
    double time_sum = 0;
    double num_correct = 0;
    for (int i=0; i<len; i++) {
        time_sum += times[i];
        if (correct[i]) {
            num_correct += 1;
        }
    }
    Stats stats = {len/time_sum*60.0, num_correct/len};
    return stats;
}

Uint32 idozit(Uint32 ms, void* param) {
    SDL_Event event;
    event.type = *(SDL_EventType*)param;
    SDL_PushEvent(&event);
    return ms;
}

void run_game(GameData* game_data, Text text, SDL_Rect* word_rects, int btn_W, int btn_H, int btn_top, int input_top, SDL_Rect countdown_box, int text_top, int kocsi_margo, int dx, int car_right) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    TTF_Font* underlined = game_data->underlined;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color zold = {0, 255, 0};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color vilagos_piros = {255, 114, 118};
    //a statisztikához használt változók:
    bool* correct = (bool*) malloc(sizeof(bool)*text.word_count);
    double* times = (double*) malloc(sizeof(double)*text.word_count);
    for (int i=0; i<text.word_count; i++) {
        correct[i] = true;
        times[i] = 0.0;
    }
    //ahova a szöveg íródik majd:
    SDL_Rect input_box = {game_data->margo, input_top, game_data->szeles-2*game_data->margo, game_data->margo};
    Button menu_button = {{game_data->szeles-btn_W-2*game_data->margo, btn_top, btn_W, btn_H}, feher, fekete, "Menu", go_to_menu};
    Button settings_button = {{2*game_data->margo, btn_top, btn_W, btn_H}, feher, fekete, "Settings", go_to_settings};
    const int num_buttons = 2; //csak 2 gomb: menübe lépés, és beállításokba lépés
    Button buttons[2] = {menu_button, settings_button};
    //az írogatáshoz használt segédváltozók
    char input[HOSSZ] = "";
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE] = "";
    char textandcomposition[HOSSZ + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1] = "";
    int i=0;
    bool draw = true;
    bool quit = false;
    bool countdown_over = false;
    int countdown = 5;
    SDL_StartTextInput();
    SDL_Event event;
    clock_t t;
    while (!quit && (game_data->game_view == BotGame || game_data->game_view == SingleGame ||game_data->game_view == MultiGame) && i <text.word_count && SDL_WaitEvent(&event)) {
        char* target = text.words[i];
        if (countdown_over) {
            if (game_data->game_view == BotGame) {
                for (int j=0; j<BOT_NUM; j++) {
                    if (game_data->bots[j].tick == event.type) {
                        game_data->bots[j].car.x = game_data->bots[j].car.x+dx < car_right? game_data->bots[j].car.x+dx : car_right;
                        draw = true;
                    }
                }
            }
            if (game_data->game_view == MultiGame) {
                for (int j=0; j<game_data->players; j++) {
                    if (game_data->multis[j].active && game_data->multis[j].tick == event.type) {
                        game_data->multis[j].car.x = game_data->multis[j].car.x+dx < car_right ? game_data->multis[j].car.x+dx : car_right;
                        draw = true;
                    }
                }
            }
        }
        switch (event.type) {
            case TICK_SEC:
                if (!countdown_over) {
                    draw = true;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int j=0; j<num_buttons; j++) {
                        if (in_rect(buttons[j].rect, event.button.x, event.button.y)) {
                            buttons[j].func(game_data);
                            draw = true;
                            break;
                        }
                    }
                }
                break;
            /* Kulonleges karakter */
             case SDL_KEYDOWN:
                 if (countdown_over) {
                     if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        handle_backspace(input);
                        draw = true;
                     }
                     if (event.key.keysym.sym == SDLK_SPACE) {
                        handle_space(input, target, composition, event);
                        draw = true;
                     }
                 }
                 break;
            case SDL_TEXTINPUT:
                if (countdown_over) {
                    /* A feldolgozott szoveg bemenete */
                    handle_textinput(input, composition, event);
                    draw = true;
                 }
                 break;
            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                if (countdown_over) {
                    strcpy(composition, event.edit.text);
                    draw = true;
                 }
                 break;
            case NEXT_WORD_EVENT:
                times[i] = ((double)(clock() - t))/CLOCKS_PER_SEC;
                t = clock();
                i++;
                game_data->player_car.x += dx;
                if (i == text.word_count) {
                    break;
                }
                SDL_StartTextInput();
                input[0] = '\0';
                composition[0] = '\0';
                draw = true;
                break;
            case SDL_QUIT:
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
        if (draw) {
            clear_screen(game_data, vilagos_kek);
            bool inputCorrect = input_correct(target, input);
            SDL_Color inputColor = inputCorrect ? feher : vilagos_piros;
            if (!inputCorrect && correct[i]) {
                correct[i] = false;
            }
            render_input(input, input_box, inputColor , font, renderer, composition, textandcomposition);
            render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
            if (game_data->game_view == BotGame) {
                for (int j=0; j<BOT_NUM; j++) {
                    render_car(renderer, game_data->bots[j].car, font);
                }
            }
            if (game_data->game_view == MultiGame) {
                for (int j=0; j<game_data->players; j++) {
                    if (strcmp(game_data->player_car.name, game_data->multis[j].car.name) != 0) {
                        render_car(renderer, game_data->multis[j].car, font);
                    }
                }
            }
            render_car(renderer, game_data->player_car, font);
            for (int j=0; j<num_buttons; j++) {
                render_button(buttons[j], renderer, font);
            }
            if (!countdown_over) {
                handle_countdown_s(&countdown_over, countdown_box, &countdown, font, &t, renderer);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
    SDL_StopTextInput();
    game_data->stats = calculate_stats(text.word_count, correct, times);
    free(correct);
    free(times);
}

void run_single_game(GameData* game_data) {
    TextArray* textarray = game_data->p_textarray;
    Text text = textarray->texts[rand()%textarray->text_count];
    TTF_Font* font = game_data->font;
    game_data->players = 1;
    //a felhasznált timer-ek:
    SDL_EventType tick_sec = TICK_SEC;
    SDL_TimerID sec_tick = SDL_AddTimer(1000, idozit, &tick_sec);
    const int N = game_data->players; //ennyi kocsi lesz a képernyõn, ettõl függõen kell elrendezni a UI-t
    const int btn_H = 80;
    const int btn_W = 150;
    //UI elrendezés_
    int btn_top = game_data->magas - btn_H - (int)((5.0/4.0 - 1.0/4.0*(double)N)*game_data->margo);
    int input_top = btn_top - (int)((-0.125*N + 2.125)*game_data->margo);
    SDL_Rect countdown_box = {game_data->szeles/2-1.6*btn_H, input_top-btn_H, 3.2*btn_H, 0.8*btn_H};
    int text_top = countdown_box.y - 1.5*btn_H;
    SDL_Rect* word_rects = calculate_Rects(text, font, game_data->margo, text_top, game_data->szeles-2*game_data->margo);
    int kocsi_margo = (int)((-0.2*N + 1.2)*game_data->margo);
    int dy = (text_top-2*kocsi_margo)/N;
    int car_h = dy*0.85;
    int car_w = car_h*2;
    int car_left = game_data->margo*2;
    int car_right = game_data->szeles-game_data->margo-car_w;
    int dx = (car_right-car_left)/text.word_count;
    game_data->player_car.x = car_left;
    game_data->player_car.y = kocsi_margo;
    game_data->player_car.w = car_w;
    game_data->player_car.h = car_h;
    strcpy(game_data->player_car.name, " ");
    run_game(game_data, text, word_rects, btn_W, btn_H, btn_top, input_top, countdown_box, text_top, kocsi_margo, dx, car_right);
    SDL_RemoveTimer(sec_tick);
    if (game_data->game_view == SingleGame) {
        game_data->game_view = Statistics;
        SDL_Event event;
        event.type = GAME_VIEW_CHANGED_EVENT;
        SDL_PushEvent(&event);
    }
    free(word_rects);
}

void run_bot_game(GameData* game_data) {
    TextArray* textarray = game_data->p_textarray;
    Text text = textarray->texts[rand()%textarray->text_count];
    TTF_Font* font = game_data->font;
    game_data->players = 5;
    randomize_bots(game_data);
    //a felhasznált timer-ek:
    SDL_EventType tick_sec = TICK_SEC;
    SDL_TimerID sec_tick = SDL_AddTimer(1000, idozit, &tick_sec);
    SDL_TimerID timer_ids[BOT_NUM];
    const int N = game_data->players; //ennyi kocsi lesz a képernyõn, ettõl függõen kell elrendezni a UI-t
    const int btn_H = 80;
    const int btn_W = 150;
    //UI elrendezés_
    int btn_top = game_data->magas - btn_H - (int)((5.0/4.0 - 1.0/4.0*(double)N)*game_data->margo);
    int input_top = btn_top - (int)((-0.125*N + 2.125)*game_data->margo);
    SDL_Rect countdown_box = {game_data->szeles/2-1.6*btn_H, input_top-btn_H, 3.2*btn_H, 0.8*btn_H};
    int text_top = countdown_box.y - 1.5*btn_H;
    SDL_Rect* word_rects = calculate_Rects(text, font, game_data->margo, text_top, game_data->szeles-2*game_data->margo);
    int kocsi_margo = (int)((-0.2*N + 1.2)*game_data->margo);
    int dy = (text_top-2*kocsi_margo)/N;
    int car_h = dy*0.85;
    int car_w = car_h*2;
    int car_left = game_data->margo*2;
    int car_right = game_data->szeles-game_data->margo-car_w;
    int dx = (car_right-car_left)/text.word_count;
    for (int i=0; i<BOT_NUM; i++) {
        timer_ids[i] = SDL_AddTimer(game_data->bots[i].ms, idozit, &(game_data->bots[i].tick));
        game_data->bots[i].car.x = car_left;
        game_data->bots[i].car.y = kocsi_margo + i*dy;
        game_data->bots[i].car.w = car_w;
        game_data->bots[i].car.h = car_h;
    }
    game_data->player_car.x = car_left;
    game_data->player_car.y = kocsi_margo + BOT_NUM * dy;
    game_data->player_car.w = car_w;
    game_data->player_car.h = car_h;
    strcpy(game_data->player_car.name, "You");
    run_game(game_data, text, word_rects, btn_W, btn_H, btn_top, input_top, countdown_box, text_top, kocsi_margo, dx, car_right);
    SDL_RemoveTimer(sec_tick);
    for (int i=0; i<BOT_NUM; i++) {
        game_data->bots[i].active = false;
        SDL_RemoveTimer(timer_ids[i]);
    }
    if (game_data->game_view == BotGame) {
        game_data->game_view = Statistics;
        SDL_Event event;
        event.type = GAME_VIEW_CHANGED_EVENT;
        SDL_PushEvent(&event);
    }
    free(word_rects);
}

void wait_for_keypress() {
    SDL_Event event;
    bool quit = false;
    while (!quit && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                quit = true;
                return;
            case SDL_QUIT:
                quit=true;
                SDL_PushEvent(&event);
                return;
        }
    }
}

int compare_wpm(const void *a, const void *b) {
    return (int)((Bot*)b)->expected_wpm - ((Bot*)a)->expected_wpm;
}

void run_multi_game(GameData* game_data) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    TextArray* textarray = game_data->p_textarray;
    Text text = textarray->texts[rand()%textarray->text_count];
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color szurke = {195, 195, 195};
    game_data->players = 0;
    do {
        clear_screen(game_data, vilagos_kek);
        char display_str[2*HOSSZ];
        sprintf(display_str, "Enter player name, or ESC for Menu!");
        render_string_blended(display_str, fekete, font, game_data->szeles/4, game_data->magas/2-2*game_data->margo, renderer, Middle);
        render_players(game_data, fekete, szurke);
        SDL_RenderPresent(renderer);
        SDL_Rect input_box = {game_data->margo, game_data->magas/2, game_data->szeles/2-2*game_data->margo, game_data->margo};
        char input[HOSSZ];
        bool esc;
        if (!input_text(input, HOSSZ, input_box, feher, fekete, font, renderer, &esc)) {
            if (esc) {
                go_to_menu(game_data);
                return;
            }
            break;
        }
        if (input[0] != '\0') {
            game_data->players++;
            game_data->multis = (Bot*) realloc(game_data->multis, sizeof(Bot)*game_data->players);
            SDL_Color new_color = {rand()%255, rand()%255, rand()%255};
            Car new_car = {0, 0, 0, 0, new_color, fekete, ""};
            strcpy(new_car.name, input);
            Bot new_player = {0, 0, new_car, TICK_SEC+BOT_NUM+game_data->players, false};
            game_data->multis[game_data->players-1] = new_player;
        } else if (game_data->players < 2) {
            printf("Need more players!\n");
        } else {
            break;
        }
    } while (game_data->players < 5);
    SDL_EventType tick_sec = TICK_SEC;
    SDL_TimerID sec_tick = SDL_AddTimer(1000, idozit, &tick_sec);
    const int N = game_data->players; //ennyi nem-player lesz a képernyõn, ettõl függõen kell elrendezni a UI-t
    const int btn_H = 80;
    const int btn_W = 150;
    //UI elrendezés_
    int btn_top = game_data->magas - btn_H - (int)((5.0/4.0 - 1.0/4.0*(double)N)*game_data->margo);
    int input_top = btn_top - (int)((-0.125*N + 2.125)*game_data->margo);
    SDL_Rect countdown_box = {game_data->szeles/2-1.6*btn_H, input_top-btn_H, 3.2*btn_H, 0.8*btn_H};
    int text_top = countdown_box.y - 1.5*btn_H;
    SDL_Rect* word_rects = calculate_Rects(text, font, game_data->margo, text_top, game_data->szeles-2*game_data->margo);
    int kocsi_margo = (int)((-0.2*N + 1.2)*game_data->margo);
    int dy = (text_top-2*kocsi_margo)/N;
    int car_h = dy*0.85;
    int car_w = car_h*2;
    int car_left = game_data->margo*2;
    int car_right = game_data->szeles-game_data->margo-car_w;
    int dx = (car_right-car_left)/text.word_count;
    game_data->player_car.y = kocsi_margo + (N-1) * dy;
    game_data->player_car.w = car_w;
    game_data->player_car.h = car_h;
    SDL_TimerID timer_ids[BOT_NUM];
    for (int i=0; i<game_data->players && game_data->game_view == MultiGame; i++) {
        int y=kocsi_margo;
        strcpy(game_data->player_car.name, game_data->multis[i].car.name);
        for (int j=0; j<game_data->players; j++) {
            game_data->multis[j].car.x = car_left;
            game_data->multis[j].car.y = y;
            game_data->multis[j].car.w = car_w;
            game_data->multis[j].car.h = car_h;
            if (j != i) {
                y += dy;
            }
        }
        game_data->player_car.x = car_left;
        for (int j=0; j<i; j++) {
            timer_ids[j] = SDL_AddTimer(game_data->multis[j].ms, idozit, &(game_data->multis[j].tick));
            game_data->multis[j].car.x = car_left;
        }
        //-----------------------------------------
        run_game(game_data, text, word_rects, btn_W, btn_H, btn_top, input_top, countdown_box, text_top, kocsi_margo, dx, car_right);
        //-----------------------------------------
        game_data->multis[i].expected_wpm = game_data->stats.wpm;
        game_data->multis[i].ms = 60000/game_data->stats.wpm;
        game_data->multis[i].active = true;
        for (int j=0; j<i; j++) {
            SDL_RemoveTimer(timer_ids[j]);
        }
        clear_screen(game_data, vilagos_kek);
        char display_str[2*HOSSZ];
        sprintf(display_str, "WPM: %.2f. Accuracy: %.2f%%. Press any key to continue.", game_data->stats.wpm, game_data->stats.accuracy*100);
        render_string_blended(display_str, fekete, font, game_data->szeles/2, game_data->magas/2, renderer, Middle);
        SDL_RenderPresent(renderer);
        if (game_data->game_view == MultiGame) {
            wait_for_keypress();
        }
    }
    if (game_data->game_view == MultiGame) {
        qsort(game_data->multis, game_data->players, sizeof(Bot), compare_wpm);
        for (int i=0; i<game_data->players; i++) {
            printf("%d.: %s, wpm: %.2f\n", i+1, game_data->multis[i].car.name, game_data->multis[i].expected_wpm);
            game_data->game_view = MultiStatistics;
            SDL_Event event;
            event.type = GAME_VIEW_CHANGED_EVENT;
            SDL_PushEvent(&event);
        }

    }
    SDL_RemoveTimer(sec_tick);
    free(word_rects);
}

void main_menu(GameData* game_data) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    int W = 500;
    int H = 100;
    const int num_buttons = 4;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color piros = {220, 50, 50};
    Button settings_button = {{game_data->szeles/2-W/2, 90, W, H}, feher, fekete, "Settings & Leaderboard", go_to_settings};
    Button single_game_button = {{game_data->szeles/2-W/2, settings_button.rect.y+H+30, W, H}, feher, fekete, "Practice", go_to_single_game};
    Button bot_game_button = {{game_data->szeles/2-W/2, single_game_button.rect.y+H+30, W, H}, feher, fekete, "Play vs Bots", go_to_bot_game};
    Button multi_game_button = {{game_data->szeles/2-W/2, bot_game_button.rect.y+H+30, W, H}, feher, fekete, "Multiplayer", go_to_multi_game};
    Button buttons[4] = {settings_button, single_game_button, bot_game_button, multi_game_button};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && game_data->game_view == MainMenu && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<num_buttons; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_data);
                            draw = true;
                            break;
                        }
                    }
                }
                break;
            case SDL_QUIT:
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
        if (draw) {
            clear_screen(game_data, vilagos_kek);
            render_string_blended("TYPERACER", piros, game_data->title, game_data->szeles/2, 45, renderer, Middle);
            for (int i=0; i<num_buttons; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

void settings(GameData* game_data) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font =  game_data->font;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color szurke = {195, 195, 195};
    Button menu_button = {{game_data->szeles*0.4-160, game_data->margo, 160, 60}, feher, fekete, "Menu", go_to_menu};
    Button reset_leader_button = {{game_data->szeles - game_data->margo-160, game_data->margo, 160, 60}, feher, fekete, "Reset", reset_leaderboard};
    const int num_buttons = 10;
    Button bot1_up_btn = {{game_data->szeles*0.4-40, 118, 40, 40}, feher, fekete, "+", bot1_up};
    Button bot2_up_btn = {{game_data->szeles*0.4-40, 162, 40, 40}, feher, fekete, "+", bot2_up};
    Button bot3_up_btn = {{game_data->szeles*0.4-40, 206, 40, 40}, feher, fekete, "+", bot3_up};
    Button bot4_up_btn = {{game_data->szeles*0.4-40, 250, 40, 40}, feher, fekete, "+", bot4_up};
    Button bot1_down_btn = {{game_data->margo, 118, 40, 40}, feher, fekete, "-", bot1_down};
    Button bot2_down_btn = {{game_data->margo, 162, 40, 40}, feher, fekete, "-", bot2_down};
    Button bot3_down_btn = {{game_data->margo, 206, 40, 40}, feher, fekete, "-", bot3_down};
    Button bot4_down_btn = {{game_data->margo, 250, 40, 40}, feher, fekete, "-", bot4_down};
    Button buttons[10] = {menu_button, reset_leader_button, bot1_up_btn, bot2_up_btn, bot3_up_btn, bot4_up_btn,
                          bot1_down_btn, bot2_down_btn, bot3_down_btn, bot4_down_btn};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && game_data->game_view == Settings && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<num_buttons; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_data);
                            draw = true;
                            break;
                        }
                    }
                }
                break;
            case SDL_QUIT:
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
        if (draw) {
            clear_screen(game_data, vilagos_kek);
            render_leaderboard(game_data, fekete, szurke);
            render_settings(game_data, fekete, szurke);
            for (int i=0; i<num_buttons; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}


void multi_statistics(GameData* game_data) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    Stats stats = game_data->stats;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    clear_screen(game_data, vilagos_kek);
    char display_str[2*HOSSZ];
    for (int i=0; i<game_data->players; i++) {
        sprintf(display_str, "%d.: %s, WPM: %.2f. ", i, game_data->multis[i].car.name, game_data->multis[i].expected_wpm);
        if (top10(game_data->leaderboard, game_data->multis[i].expected_wpm)) {
            strcat(display_str, "Earned a Leaderboard spot!");
            LeaderboardEntry entry;
            entry.wpm = game_data->multis[i].expected_wpm;
            strcpy(entry.name, game_data->multis[i].car.name);
            update_leaderboard(&(game_data->leaderboard), entry);
        }
        render_string_blended(display_str, fekete, font, game_data->szeles/2, game_data->margo*(i+2), renderer, Middle);
    }
    sprintf(display_str, "WPM: %.2f. Accuracy: %.2f%%.", stats.wpm, stats.accuracy*100);
    Button menu_button = {{game_data->szeles/2-100, game_data->magas-150, 200, 100}, feher, fekete, "Menu", go_to_menu};
    Button settings_button = {{menu_button.rect.x, menu_button.rect.y - menu_button.rect.h-game_data->margo, menu_button.rect.w, menu_button.rect.h}, feher, fekete, "Settings", go_to_settings};
    Button buttons[2] = {menu_button, settings_button};
    for (int i=0; i<2; i++) {
        render_button(buttons[i], renderer, font);
    }
    SDL_RenderPresent(renderer);
    bool quit = false;
    SDL_Event event;
    while (!quit && game_data->game_view == MultiStatistics && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<2; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_data);
                            break;
                        }
                    }
                }
                break;
            case SDL_QUIT:
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
    }
}

void statistics(GameData* game_data) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    Stats stats = game_data->stats;
    LeaderBoard leaderboard = game_data->leaderboard;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    char display_str[2*HOSSZ];
    sprintf(display_str, "WPM: %.2f. Accuracy: %.2f%%.", stats.wpm, stats.accuracy*100);
    Button menu_button = {{game_data->szeles/2-100, game_data->magas-150, 200, 100}, feher, fekete, "Menu", go_to_menu};
    Button settings_button = {{menu_button.rect.x, menu_button.rect.y - menu_button.rect.h-game_data->margo, menu_button.rect.w, menu_button.rect.h}, feher, fekete, "Settings", go_to_settings};
    if (top10(leaderboard, stats.wpm)) {
        settings_button.str = "Enter Name";
        settings_button.func = get_name;
        strcat(display_str, " Eligible for Leaderboard!!");
    }
    Button buttons[2] = {menu_button, settings_button};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && game_data->game_view == Statistics && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<2; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_data);
                            draw = true;
                            break;
                        }
                    }
                }
                break;
            case SDL_QUIT:
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
        if (draw) {
            clear_screen(game_data, vilagos_kek);
            render_string_blended(display_str, fekete, font, game_data->szeles/2, settings_button.rect.y - game_data->margo, renderer, Middle);
            for (int i=0; i<2; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

void ask_name(GameData* game_data) {
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    clear_screen(game_data, vilagos_kek);
    char display_str[2*HOSSZ];
    sprintf(display_str, "WPM: %.2f, enter name!", game_data->stats.wpm);
    render_string_blended(display_str, fekete, font, game_data->szeles/2, game_data->magas/2-2*game_data->margo, renderer, Middle);
    SDL_RenderPresent(renderer);
    SDL_Rect input_box = {game_data->margo, game_data->magas/2, game_data->szeles-2*game_data->margo, game_data->margo};
    char input[HOSSZ];
    bool esc;
    if (input_text(input, HOSSZ, input_box, feher, fekete, font, renderer, &esc)) {
        printf("Got this: %s\n", input);
        LeaderboardEntry entry;
        entry.wpm = game_data->stats.wpm;
        strcpy(entry.name, input);
        update_leaderboard(&(game_data->leaderboard), entry);
    }
    go_to_menu(game_data);
}

int main(int argc, char *argv[]) {
    srand(time(0)); //inicializáljuk a randomszám generátort
    const int magas = 600;
    const int szeles = 1000;
    const int margo = 40;
    Bot bots[BOT_NUM];
    init_bots(bots);
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    TTF_Font *title;
    sdl_init(szeles, magas, FONT, &window, &renderer, &font, &underlined, &title);
    TextArray textarray = parse_file("hobbit_short.txt");
    LeaderBoard leaderboard;
    if (!load_leaderboard(&leaderboard)) {
        printf("Could not load leaderboard.\n");
    }
    Car player_car = {0, 0, 0, 0, {255, 0, 0, 0}, {0, 0, 0}, "You"};
    GameView game_view = MainMenu;
    GameData game_data = {{0, 0}, leaderboard, renderer, font, underlined, title, &textarray, game_view, magas, szeles, margo, bots, NULL, 0, player_car};
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
    bool quit = false;
    while (!quit && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case GAME_VIEW_CHANGED_EVENT:
                switch (game_data.game_view){
                    case MainMenu:
                        main_menu(&game_data);
                        break;
                    case Settings:
                        settings(&game_data);
                        break;
                    case SingleGame:
                        run_single_game(&game_data);
                        break;
                    case BotGame:
                        run_bot_game(&game_data);
                        break;
                    case MultiGame: //TODO
                        run_multi_game(&game_data);
                        break;
                    case Statistics:
                        statistics(&game_data);
                        break;
                    case MultiStatistics:
                        multi_statistics(&game_data);
                        break;
                    case AskName:
                        ask_name(&game_data);
                        break;
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
        }
    }
    free(game_data.multis);
    game_data.multis = NULL;
    save_leaderboard(game_data.leaderboard);
    free_textarray(&textarray);
    sdl_close(&window, &renderer, &font, &underlined, &title);
    return 0;
}
