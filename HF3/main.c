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


/*A programozás alapjai 1 házi feladat: Typeracer
Gaál Botond
CRQEYD
2023.11.12.*/

/*Az osszes SDL-el kapcsolatos dolog inicializálása:
-ablak nyitás
-renderer készítés
-font megnyitása
*/

typedef enum {
    MainMenu = 0,
    SingleGame = 1,
    Settings = 2,
    Statistics = 3,
} GameView;

typedef struct Stats {
    double wpm;
    double accuracy;
} Stats;

void sdl_init(int szeles, int magas, const char* tipus, SDL_Window** pwindow, SDL_Renderer** prenderer, TTF_Font** pfont, TTF_Font** punderlined) {
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
    TTF_Font *font = TTF_OpenFont(tipus, 32);
    if (font == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_Font *underlined = TTF_OpenFont(tipus, 32);
    if (underlined == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_SetFontStyle(underlined, TTF_STYLE_UNDERLINE);
    *pwindow = window;
    *prenderer = renderer;
    *pfont = font;
    *punderlined = underlined;
    #ifdef __WIN32__
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    #endif
}

void sdl_close(SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **pfont) {
    SDL_DestroyRenderer(*prenderer);
    *prenderer = NULL;

    SDL_DestroyWindow(*pwindow);
    *pwindow = NULL;

    TTF_CloseFont(*pfont);
    *pfont = NULL;

    SDL_Quit();
}

void handle_backspace(char* input) {
    int textlen = strlen(input);
    do {
        if (textlen == 0) {
            break;
        }
        if ((input[textlen-1] & 0x80) == 0x00)   {
            /* Egy bajt */
            input[textlen-1] = 0x00;
            break;
        }
        if ((input[textlen-1] & 0xC0) == 0x80) {
            /* Bajt, egy tobb-bajtos szekvenciabol */
            input[textlen-1] = 0x00;
            textlen--;
        }
        if ((input[textlen-1] & 0xC0) == 0xC0) {
            /* Egy tobb-bajtos szekvencia elso bajtja */
            input[textlen-1] = 0x00;
            break;
        }
    } while(true);

}

void handle_space(char* input, char* target, char* composition, SDL_Event event) {
    if (event.key.keysym.sym == SDLK_SPACE) {
        if (strcmp(input, target)==0) {
            SDL_StopTextInput();
            SDL_Event next;
            next.type = NEXT_WORD_EVENT;
            SDL_PushEvent(&next);
        }
    }
}

void handle_textinput(char* input, char* composition, SDL_Event event) {
    /* A feldolgozott szoveg bemenete */
    if (strlen(input) + strlen(event.text.text) < HOSSZ) {
        strcat(input, event.text.text);
    }
    /* Az eddigi szerkesztes torolheto */
    composition[0] = '\0';
}

bool in_rect(SDL_Rect rect, int x, int y) {
    bool X = (x >= rect.x) && (x <= rect.x + rect.w);
    bool Y = (y >= rect.y) && (y <= rect.y + rect.h);
    return X && Y;
}

typedef struct Button {
    SDL_Rect rect;
    SDL_Color color;
    SDL_Color str_color;
    char* str;
    void (*func)(GameView*);
} Button;

void go_to_game(GameView* gameview) {
    printf("Clicked GAME\n");
    *gameview = SingleGame;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void go_to_menu(GameView* gameview) {
    printf("Clicked MENU\n");
    *gameview = MainMenu;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void go_to_settings(GameView* gameview) {
    printf("Clicked SETTINGS\n");
    *gameview = Settings;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void render_button(Button button, SDL_Renderer* renderer, TTF_Font *font) {
    SDL_Rect text_rect;
    text_rect.x = button.rect.x + button.rect.w * 0.1;
    text_rect.y = button.rect.y + button.rect.h * 0.1;
    text_rect.w = button.rect.w * 0.8;
    text_rect.h = button.rect.h * 0.8;
    boxRGBA(renderer, button.rect.x, button.rect.y, button.rect.x + button.rect.w, button.rect.y + button.rect.h, button.color.r, button.color.g, button.color.b, 255);
    rectangleRGBA(renderer, button.rect.x, button.rect.y, button.rect.x + button.rect.w, button.rect.y + button.rect.h, 0, 0, 0, 255);
    render_string_to_rect_blended(button.str, button.str_color, font, text_rect, renderer);
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
    Stats stats = {time_sum/len*60.0, num_correct/len};
    return stats;
}

Uint32 idozit(Uint32 ms, void* param) {
    SDL_Event event;
    event.type = S_TIMER_TICK;
    SDL_PushEvent(&event);
    return ms;
}

void run_single_game(TextArray* textarray, GameView* game_view, SDL_Renderer *renderer, TTF_Font *font, TTF_Font *underlined, Stats* stats) { //should eventually return statistics instead of void
     /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color zold = {0, 255, 0};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color vilagos_piros = {255, 114, 118};
    SDL_Color sotet_piros = {112, 57, 63};
    Text text = textarray->texts[rand()%textarray->text_count];
    bool* correct = (bool*) malloc(sizeof(bool)*text.word_count);
    double* times = (double*) malloc(sizeof(double)*text.word_count);
    for (int i=0; i<text.word_count; i++) {
        correct[i] = true;
        times[i] = 0.0;
    }
    SDL_Rect input_box = {MARGO, MAGAS/2, SZELES-5*MARGO, MARGO};
    SDL_Rect menu_button_rect = {SZELES-3*MARGO, MAGAS/2, 3*MARGO, 1.5*MARGO};
    Button menu_button = {menu_button_rect, feher, fekete, "MENU", go_to_menu};
    char input[HOSSZ] = "";
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE] = "";
    char textandcomposition[HOSSZ + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1] = "";
    SDL_Rect* word_rects = calculate_Rects(text, font, MARGO, MARGO, SZELES-2*MARGO);
    int i=0;
    bool draw = true;
    bool quit = false;
    bool countdown_over = false;
    int N=10;
    SDL_TimerID id = SDL_AddTimer(1000, idozit, NULL);
    SDL_StartTextInput();
    SDL_Event event;
    clock_t t = clock();
    while (!quit && *game_view == SingleGame && i <text.word_count && SDL_WaitEvent(&event)) {
        char* target = text.words[i];
        switch (event.type) {
            case S_TIMER_TICK:
                if (N > 1) {
                    N--;
                    draw = true;
                } else {
                    countdown_over = true;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT && in_rect(menu_button.rect, event.button.x, event.button.y)) {
                    menu_button.func(game_view);
                    draw = true;
                }
                break;
            /* Kulonleges karakter */
             case SDL_KEYDOWN:
                 if (!countdown_over) {
                    break;
                 }
                 if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    handle_backspace(input);
                    draw = true;
                 }
                 if (event.key.keysym.sym == SDLK_SPACE) {
                    handle_space(input, target, composition, event);
                    draw = true;
                 }
                 break;
            case SDL_TEXTINPUT:
                if (!countdown_over) {
                    break;
                 }
                /* A feldolgozott szoveg bemenete */
                handle_textinput(input, composition, event);
                draw = true;
                break;
            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                if (!countdown_over) {
                    break;
                 }
                strcpy(composition, event.edit.text);
                draw = true;
                break;
            case NEXT_WORD_EVENT:
                times[i] = ((double)(clock() - t))/CLOCKS_PER_SEC;
                t = clock();
                i++;
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
            clear_screen(renderer, vilagos_kek);
            bool inputCorrect = input_correct(target, input);
            SDL_Color inputColor = inputCorrect ? feher : vilagos_piros;
            if (!inputCorrect && correct[i]) {
                correct[i] = false;
            }
            render_input(input, input_box, inputColor , font, renderer, composition, textandcomposition);
            render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
            render_car(renderer, sotet_piros, fekete, MARGO+(SZELES-KOCSI_W-MARGO)*i/text.word_count, MAGAS-KOCSI_H, KOCSI_W, KOCSI_H);
            render_button(menu_button, renderer, font);
            if (!countdown_over) { //replace this with something nice
                if (N>5){
                    boxRGBA(renderer, SZELES/2-N*MARGO/2, MAGAS/2-2*MARGO, SZELES/2+N*MARGO/2, MAGAS/2-MARGO, 255, 50, 50, 255);
                } else if (N>1) {
                    boxRGBA(renderer, SZELES/2-N*MARGO/2, MAGAS/2-2*MARGO, SZELES/2+N*MARGO/2, MAGAS/2-MARGO, 255, 255, 0, 255);
                } else {
                    boxRGBA(renderer, SZELES/2-N*MARGO/2, MAGAS/2-2*MARGO, SZELES/2+N*MARGO/2, MAGAS/2-MARGO, 0, 255, 0, 255);
                }

            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
    SDL_StopTextInput();
    *stats = calculate_stats(text.word_count, correct, times);
    free(correct);
    free(times);
    free(word_rects);
    SDL_RemoveTimer(id);
    if (*game_view == SingleGame) {
        *game_view = Statistics;
        event.type = GAME_VIEW_CHANGED_EVENT;
        SDL_PushEvent(&event);
    }
}

void main_menu(GameView* game_view, SDL_Renderer *renderer, TTF_Font *font) {
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Rect settings_button_rect = {SZELES/2-100, MAGAS/2-110, 200, 100};
    Button settings_button = {settings_button_rect, feher, fekete, "Settings", go_to_settings};
    SDL_Rect game_button_rect = {SZELES/2-100, MAGAS/2+10, 200, 100};
    Button game_button = {game_button_rect, feher, fekete, "Game", go_to_game};
    Button buttons[2] = {settings_button, game_button};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && *game_view == MainMenu && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<2; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_view);
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
            clear_screen(renderer, vilagos_kek);
            for (int i=0; i<2; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

void settings(GameView* game_view, SDL_Renderer *renderer, TTF_Font *font) {
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Rect menu_button_rect = {SZELES/2-100, MAGAS/2-50, 200, 100};
    Button menu_button = {menu_button_rect, feher, fekete, "Menu", go_to_menu};
    Button buttons[1] = {menu_button};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && *game_view == Settings && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<1; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_view);
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
            clear_screen(renderer, vilagos_kek);
            for (int i=0; i<1; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

void statistics(GameView* game_view, SDL_Renderer *renderer, TTF_Font *font, Stats stats) {
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Rect menu_button_rect = {SZELES/2-100, MAGAS-150, 200, 100};
    Button menu_button = {menu_button_rect, feher, fekete, "Menu", go_to_menu};
    char stats_str[10*HOSSZ];
    sprintf(stats_str, "Az átlagos WPM-ed: %.2f. Az átlagos pontosságod: %.2f%%", stats.wpm, stats.accuracy*100);
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && *game_view == Statistics && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT && in_rect(menu_button.rect, event.button.x, event.button.y)) {
                    menu_button.func(game_view);
                    draw = true;
                }
                break;
            case SDL_QUIT:
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
        if (draw) {
            clear_screen(renderer, vilagos_kek);
            render_string_blended(stats_str, fekete, font, MARGO, MARGO, renderer);
            render_button(menu_button, renderer, font);
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

int main(int argc, char *argv[]) {
    srand(time(0)); //inicializáljuk a randomszám generátort
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    Stats stats;
    sdl_init(SZELES, MAGAS, FONT ,&window, &renderer, &font, &underlined);
    TextArray textarray = parse_file("hobbit_short.txt");
    GameView game_view = MainMenu;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
    bool quit = false;
    while (!quit && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case GAME_VIEW_CHANGED_EVENT:
                switch (game_view){
                    case MainMenu:
                        main_menu(&game_view, renderer, font);
                        break;
                    case Settings:
                        settings(&game_view, renderer, font);
                        break;
                    case SingleGame:
                        run_single_game(&textarray, &game_view, renderer, font, underlined, &stats);
                        break;
                    case Statistics:
                        statistics(&game_view, renderer, font, stats);
                        break;
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
        }
    }
    free_textarray(&textarray);
    sdl_close(&window, &renderer, &font);
    return 0;
}
