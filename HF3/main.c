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

#define LEADERBOARD_SIZE 10
#define LEADERBOARD "leaderboard.txt"


/*A programozás alapjai 1 házi feladat: Typeracer
Gaál Botond
CRQEYD
2023.11.12.*/

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

typedef struct LeaderboardEntry {
    double wpm;
    char name[HOSSZ];
} LeaderboardEntry;

typedef struct LeaderBoard {
    LeaderboardEntry entries[LEADERBOARD_SIZE];
    int num;
} LeaderBoard;

typedef struct GameState {
    Stats stats;
    LeaderBoard leaderboard;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* underlined;
    TextArray* p_textarray;
    GameView game_view;
} GameState;

bool load_leaderboard(LeaderBoard* leaderboard) {
    FILE* file = fopen(LEADERBOARD, "r");
    if (file == NULL) {
        leaderboard->num = 0;
        return false;
    }
    fscanf(file, "%d", &(leaderboard->num));
    for (int i=0; i<leaderboard->num; i++) {
        fscanf(file, "%lf %s", &(leaderboard->entries[i].wpm), leaderboard->entries[i].name);
    }
    fclose(file);
    return true;
}

bool save_leaderboard(LeaderBoard leaderboard) {
    FILE* file = fopen(LEADERBOARD, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file for writing\n");
        return false;
    }
    fprintf(file, "%d\n", leaderboard.num);
    for (int i=0; i<leaderboard.num; i++) {
        fprintf(file, "%.2lf %s\n", leaderboard.entries[i].wpm, leaderboard.entries[i].name);
    }
    fclose(file);
    return true;
}

void add_entry(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    int position = leaderboard->num;
    for (int i=0; i < leaderboard->num; i++) {
        if (entry.wpm > leaderboard->entries[i].wpm) {
            position = i;
            break;
        }
    }
    leaderboard->num = leaderboard->num < 10 ? leaderboard->num + 1 : 10;
    for (int i=leaderboard->num-1; i > position; i--) {
        leaderboard->entries[i].wpm = leaderboard->entries[i-1].wpm;
        strcpy(leaderboard->entries[i].name, leaderboard->entries[i-1].name);
    }
    leaderboard->entries[position].wpm = entry.wpm;
    strcpy(leaderboard->entries[position].name, entry.name);
}

int find_index(LeaderBoard leaderboard, LeaderboardEntry entry) {
    for (int i=0; i<leaderboard.num; i++) {
        if (strcmp(leaderboard.entries[i].name, entry.name) == 0) {
            return i;
        }
    }
    return -1;
}

void remove_entry(LeaderBoard* leaderboard, int idx) {
    if (leaderboard->num == 1) {
        leaderboard->num = 0;
    } else {
        leaderboard->num -= 1;
        for (int i=idx; i<leaderboard->num; i++) {
            leaderboard->entries[i].wpm = leaderboard->entries[i+1].wpm;
            strcpy(leaderboard->entries[i].name, leaderboard->entries[i+1].name);
        }
    }
}

void print_leaderboard(LeaderBoard leaderboard) {
    printf("----------------\nThe leaderboard:\n");
    for (int i=0; i<leaderboard.num; i++) {
        printf("Name: %s, wpm: %.2f\n", leaderboard.entries[i].name, leaderboard.entries[i].wpm);
    }
    printf("----------------\n");
}

void update_leaderboard(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    int idx = find_index(*leaderboard, entry);
    if (idx == -1) {
        add_entry(leaderboard, entry);
    }
    if (idx != -1 && leaderboard->entries[idx].wpm < entry.wpm) {
        print_leaderboard(*leaderboard);
        remove_entry(leaderboard, idx);
        print_leaderboard(*leaderboard);
        add_entry(leaderboard, entry);
    }

}

bool top10(LeaderBoard leaderboard, double wpm) {
    double slowest = leaderboard.entries[leaderboard.num-1].wpm;
    if (leaderboard.num < LEADERBOARD_SIZE || slowest < wpm) {
        return true;
    } else {
        return false;
    }
}

void render_leaderboard(LeaderBoard leaderboard, SDL_Color color, TTF_Font* font, SDL_Renderer* renderer) {
    int top = MARGO;
    int bottom = MAGAS - MARGO;
    int left = SZELES*0.4 + MARGO;
    int right = SZELES-MARGO;
    SDL_Color szurke = {195, 195, 195};
    boxRGBA(renderer, left, top, right, bottom, szurke.r, szurke.g, szurke.b, 255);
    rectangleRGBA(renderer, left, top, right, bottom, color.r, color.g, color.b, 255);
    SDL_Rect rect = render_string_blended("Ranglista:", color, font, left, top, renderer, TopLeft);
    top += rect.h + MARGO;
    int h = (bottom - top) / LEADERBOARD_SIZE;
    for (int i=0; i<leaderboard.num; i++) {
        rectangleRGBA(renderer, left, top+h*i, right, top+h*(i+1), color.r, color.g, color.b, 255);
        char display_str[2*HOSSZ];
        sprintf(display_str, "%d.: %s, WPM: %.2f", i+1, leaderboard.entries[i].name, leaderboard.entries[i].wpm);
        render_string_blended(display_str, color, font, (left+right)/2, top+h*(i+0.5), renderer, Middle);
    }
}

/* Beolvas egy szoveget a billentyuzetrol.
 * A rajzolashoz hasznalt font es a megjelenito az utolso parameterek.
 * Az elso a tomb, ahova a beolvasott szoveg kerul.
 * A masodik a maximális hossz, ami beolvasható.
 * A visszateresi erteke logikai igaz, ha sikerult a beolvasas. */
bool input_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, TTF_Font *font, SDL_Renderer *renderer) {
    /* Ez tartalmazza az aktualis szerkesztest */
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    composition[0] = '\0';
    /* Ezt a kirajzolas kozben hasznaljuk */
    char textandcomposition[hossz + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    /* Max hasznalhato szelesseg */
    int maxw = teglalap.w - 2;
    int maxh = teglalap.h - 2;

    dest[0] = '\0';

    bool enter = false;
    bool kilep = false;

    SDL_StartTextInput();
    while (!kilep && !enter) {
        /* doboz kirajzolasa */
        boxRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, hatter.r, hatter.g, hatter.b, 255);
        rectangleRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, szoveg.r, szoveg.g, szoveg.b, 255);
        /* szoveg kirajzolasa */
        int w;
        strcpy(textandcomposition, dest);
        strcat(textandcomposition, composition);
        if (textandcomposition[0] != '\0') {
            SDL_Surface *felirat = TTF_RenderUTF8_Blended(font, textandcomposition, szoveg);
            SDL_Texture *felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
            SDL_Rect cel = { teglalap.x, teglalap.y, felirat->w < maxw ? felirat->w : maxw, felirat->h < maxh ? felirat->h : maxh };
            SDL_RenderCopy(renderer, felirat_t, NULL, &cel);
            SDL_FreeSurface(felirat);
            SDL_DestroyTexture(felirat_t);
            w = cel.w;
        } else {
            w = 0;
        }
        /* kurzor kirajzolasa */
        if (w < maxw) {
            vlineRGBA(renderer, teglalap.x + w + 2, teglalap.y + 2, teglalap.y + teglalap.h - 3, szoveg.r, szoveg.g, szoveg.b, 192);
        }
        /* megjeleniti a képernyon az eddig rajzoltakat */
        SDL_RenderPresent(renderer);

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            /* Kulonleges karakter */
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    int textlen = strlen(dest);
                    do {
                        if (textlen == 0) {
                            break;
                        }
                        if ((dest[textlen-1] & 0x80) == 0x00)   {
                            /* Egy bajt */
                            dest[textlen-1] = 0x00;
                            break;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0x80) {
                            /* Bajt, egy tobb-bajtos szekvenciabol */
                            dest[textlen-1] = 0x00;
                            textlen--;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0xC0) {
                            /* Egy tobb-bajtos szekvencia elso bajtja */
                            dest[textlen-1] = 0x00;
                            break;
                        }
                    } while(true);
                }
                if (event.key.keysym.sym == SDLK_RETURN) {
                    enter = true;
                }
                break;

            /* A feldolgozott szoveg bemenete */
            case SDL_TEXTINPUT:
                if (strlen(dest) + strlen(event.text.text) < hossz) {
                    strcat(dest, event.text.text);
                }

                /* Az eddigi szerkesztes torolheto */
                composition[0] = '\0';
                break;

            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                strcpy(composition, event.edit.text);
                break;

            case SDL_QUIT:
                /* visszatesszuk a sorba ezt az eventet, mert
                 * sok mindent nem tudunk vele kezdeni */
                SDL_PushEvent(&event);
                kilep = true;
                break;
        }
    }

    /* igaz jelzi a helyes beolvasast; = ha enter miatt allt meg a ciklus */
    SDL_StopTextInput();
    return enter;
}

/*Az osszes SDL-el kapcsolatos dolog inicializálása:
-ablak nyitás
-renderer készítés
-font megnyitása
*/
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
    void (*func)(GameState*);
} Button;

void go_to_game(GameState* game_state) {
    printf("Clicked GAME\n");
    game_state->game_view = SingleGame;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void go_to_menu(GameState* game_state) {
    printf("Clicked MENU\n");
    game_state->game_view = MainMenu;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void get_name(GameState* game_state) {
    printf("Clicked ASKNAME\n");
    game_state->game_view = AskName;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void reset_leaderboard(GameState* game_state) {
    game_state->leaderboard.num = 0;
    save_leaderboard(game_state->leaderboard);
}

void go_to_settings(GameState* game_state) {
    printf("Clicked SETTINGS\n");
    game_state->game_view = Settings;
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
    render_string_blended(button.str, button.str_color, font, text_rect.x+text_rect.w/2, text_rect.y + text_rect.h/2, renderer, Middle);
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
    event.type = TICK_SEC;
    SDL_PushEvent(&event);
    return ms;
}

void handle_countdown_s(bool* countdown_over, SDL_Rect rect, int* countdown, TTF_Font *font, clock_t* t, SDL_Renderer* renderer) {
    if (countdown >= 0) {
        SDL_Color sarga = {255, 255, 0};
        SDL_Color zold = {0, 255, 0};
        SDL_Color piros = {255, 50, 50};
        SDL_Color szurke = {195, 195, 195};
        SDL_Color sotet_szurke = {100, 100, 100};
        SDL_Color fekete = {0, 0, 0};
        SDL_Color color;
        boxRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, szurke.r, szurke.g, szurke.b, 255);
        rectangleRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, fekete.r, fekete.g, fekete.b, 255);
        char num[HOSSZ] = "";
        sprintf(num, "%.2d", *countdown);
        render_string_blended(num, fekete, font, rect.x + rect.w*7/8, rect.y + rect.h/2, renderer, Middle);
        if (*countdown > 3) {
            color = piros;
        } else {
            color = sotet_szurke;
        }
        filledCircleRGBA(renderer, rect.x + rect.w*5/8, rect.y + rect.h/2, rect.w/8.5, color.r, color.g, color.b, 255);
        circleRGBA(renderer, rect.x + rect.w*5/8, rect.y + rect.h/2, rect.w/8.5, fekete.r, fekete.g, fekete.b, 255);
        if (*countdown <= 3 && *countdown >0) {
            color = sarga;
        } else {
            color = sotet_szurke;
        }
        filledCircleRGBA(renderer, rect.x + rect.w*3/8, rect.y + rect.h/2, rect.w/8.5, color.r, color.g, color.b, 255);
        circleRGBA(renderer, rect.x + rect.w*3/8, rect.y + rect.h/2, rect.w/8.5, fekete.r, fekete.g, fekete.b, 255);
        if (*countdown == 0) {
            color = zold;
            *countdown_over = true;
            *t = clock();
        } else {
            color = sotet_szurke;
        }
        filledCircleRGBA(renderer, rect.x + rect.w/8, rect.y + rect.h/2, rect.w/8.5, color.r, color.g, color.b, 255);
        circleRGBA(renderer, rect.x + rect.w/8, rect.y + rect.h/2, rect.w/8.5, fekete.r, fekete.g, fekete.b, 255);
        *countdown -= 1;
    }
}

void run_single_game(GameState* game_state) { //should eventually return statistics instead of void
    TextArray* textarray = game_state->p_textarray;
    SDL_Renderer* renderer = game_state->renderer;
    TTF_Font* font = game_state->font;
    TTF_Font* underlined = game_state->underlined;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color zold = {0, 255, 0};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color vilagos_piros = {255, 114, 118};
    SDL_Color sotet_piros = {112, 57, 63};
    //Text text = textarray->texts[rand()%textarray->text_count];
    Text text = textarray->texts[0];
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
    int countdown = 5;
    SDL_TimerID id = SDL_AddTimer(1000, idozit, NULL);
    SDL_StartTextInput();
    SDL_Event event;
    clock_t t;
    while (!quit && game_state->game_view == SingleGame && i <text.word_count && SDL_WaitEvent(&event)) {
        char* target = text.words[i];
        switch (event.type) {
            case TICK_SEC:
                if (!countdown_over) {
                    draw = true;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT && in_rect(menu_button.rect, event.button.x, event.button.y)) {
                    menu_button.func(game_state);
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
            if (!countdown_over) {
                SDL_Rect countdown_display = {SZELES/2 - 2*MARGO, MAGAS/2 - MARGO, 4 *MARGO, MARGO};
                handle_countdown_s(&countdown_over, countdown_display, &countdown, font, &t, renderer);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
    SDL_StopTextInput();
    game_state->stats = calculate_stats(text.word_count, correct, times);
    free(correct);
    free(times);
    free(word_rects);
    SDL_RemoveTimer(id);
    if (game_state->game_view == SingleGame) {
        game_state->game_view = Statistics;
        event.type = GAME_VIEW_CHANGED_EVENT;
        SDL_PushEvent(&event);
    }
}

void main_menu(GameState* game_state) {
    SDL_Renderer* renderer = game_state->renderer;
    TTF_Font* font = game_state->font;
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
    while (!quit && game_state->game_view == MainMenu && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<2; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_state);
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

void settings(GameState* game_state) {
    SDL_Renderer* renderer = game_state->renderer;
    TTF_Font* font =  game_state->font;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    Button menu_button = {{MARGO, MAGAS/2-125, 200, 100}, feher, fekete, "Menu", go_to_menu};
    Button reset_leader_button = {{MARGO, MAGAS/2+125, 250, 100}, feher, fekete, "Reset Leaderboard", reset_leaderboard};
    Button buttons[2] = {menu_button, reset_leader_button};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && game_state->game_view == Settings && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<2; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_state);
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
            render_leaderboard(game_state->leaderboard, fekete, font, renderer);
            for (int i=0; i<2; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

void statistics(GameState* game_state) {
    //TODO: ezt meg lehetne csinálni szebben (lekezelni hogy nem tudjuk, hány gomb van)
    SDL_Renderer* renderer = game_state->renderer;
    TTF_Font* font = game_state->font;
    Stats stats = game_state->stats;
    LeaderBoard leaderboard = game_state->leaderboard;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    char display_str[10*HOSSZ];
    sprintf(display_str, "WPM: %.2f. Pontosság: %.2f%%.", stats.wpm, stats.accuracy*100);
    Button menu_button = {{SZELES/2-100, MAGAS-150, 200, 100}, feher, fekete, "Menu", go_to_menu};
    Button settings_button = {{menu_button.rect.x, menu_button.rect.y - menu_button.rect.h*2, menu_button.rect.w, menu_button.rect.h}, feher, fekete, "Settings", go_to_settings};
    if (top10(leaderboard, stats.wpm)) {
        settings_button.str = "Rögzítés";
        settings_button.func = get_name;
        strcat(display_str, " Ez ranglistás eredmény!");
    }
    Button buttons[2] = {menu_button, settings_button};
    bool quit = false;
    bool draw = true;
    SDL_Event event;
    while (!quit && game_state->game_view == Statistics && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    for (int i=0; i<2; i++) {
                        if (in_rect(buttons[i].rect, event.button.x, event.button.y)) {
                            buttons[i].func(game_state);
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
            render_string_blended(display_str, fekete, font, SZELES/2, MAGAS/2, renderer, Middle);
            for (int i=0; i<2; i++) {
                render_button(buttons[i], renderer, font);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
}

void ask_name(GameState* game_state) {
    SDL_Renderer* renderer = game_state->renderer;
    TTF_Font* font = game_state->font;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color vilagos_kek = {120, 150, 255};
    clear_screen(renderer, vilagos_kek);
    char display_str[2*HOSSZ];
    sprintf(display_str, "WPM: %.2f, adja meg a nevét hogy rögzíthessük!", game_state->stats.wpm);
    render_string_blended(display_str, fekete, font, SZELES/2, MAGAS/2-2*MARGO, renderer, Middle);
    SDL_RenderPresent(renderer);
    SDL_Rect input_box = {MARGO, MAGAS/2, SZELES-2*MARGO, MARGO};
    char input[HOSSZ];
    input_text(input, HOSSZ, input_box, feher, fekete, font, renderer);
    printf("Got this: %s\n", input);
    LeaderboardEntry entry;
    entry.wpm = game_state->stats.wpm;
    strcpy(entry.name, input);
    update_leaderboard(&(game_state->leaderboard), entry);
    print_leaderboard(game_state->leaderboard);
    game_state->game_view = Settings;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

int main(int argc, char *argv[]) {
    srand(time(0)); //inicializáljuk a randomszám generátort
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    Stats stats = {0, 0};
    sdl_init(SZELES, MAGAS, FONT ,&window, &renderer, &font, &underlined);
    TextArray textarray = parse_file("hobbit_short.txt");
    LeaderBoard leaderboard;
    if (!load_leaderboard(&leaderboard)) {
        printf("Could not load leaderboard.\n");
    } else {
        print_leaderboard(leaderboard);
    }
    GameView game_view = MainMenu;
    GameState game_state = {stats, leaderboard, renderer, font, underlined, &textarray, game_view};
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
    bool quit = false;
    while (!quit && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case GAME_VIEW_CHANGED_EVENT:
                switch (game_state.game_view){
                    case MainMenu:
                        main_menu(&game_state);
                        break;
                    case Settings:
                        settings(&game_state);
                        break;
                    case SingleGame:
                        run_single_game(&game_state);
                        break;
                    case Statistics:
                        statistics(&game_state);
                        break;
                    case AskName:
                        ask_name(&game_state);
                        break;
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
        }
    }
    save_leaderboard(game_state.leaderboard);
    free_textarray(&textarray);
    sdl_close(&window, &renderer, &font);
    return 0;
}
