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
    TTF_Font *font = TTF_OpenFont(tipus, 32);
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
    TTF_Font *underlined = TTF_OpenFont(tipus, 32);
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

bool in_rect(SDL_Rect rect, int x, int y) {
    bool X = (x >= rect.x) && (x <= rect.x + rect.w);
    bool Y = (y >= rect.y) && (y <= rect.y + rect.h);
    return X && Y;
}

void go_to_game(GameData* game_data) {
    printf("Clicked GAME\n");
    game_data->game_view = SingleGame;
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
    event.type = TICK_SEC;
    SDL_PushEvent(&event);
    return ms;
}

void run_single_game(GameData* game_data) {
    TextArray* textarray = game_data->p_textarray;
    SDL_Renderer* renderer = game_data->renderer;
    TTF_Font* font = game_data->font;
    TTF_Font* underlined = game_data->underlined;
    int W = 150;
    int H = 80;
    const int num_buttons = 2;
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color zold = {0, 255, 0};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color vilagos_piros = {255, 114, 118};
    SDL_Color sotet_piros = {112, 57, 63};
    Text text = textarray->texts[rand()%textarray->text_count];
    //Text text = textarray->texts[0];
    bool* correct = (bool*) malloc(sizeof(bool)*text.word_count);
    double* times = (double*) malloc(sizeof(double)*text.word_count);
    for (int i=0; i<text.word_count; i++) {
        correct[i] = true;
        times[i] = 0.0;
    }
    SDL_Rect input_box = {game_data->margo, game_data->magas-H-3*game_data->margo, game_data->szeles-2*game_data->margo, game_data->margo};
    SDL_Rect countdown_display = {game_data->szeles/2 - 1.6*H, input_box.y-H, 3.2*H, 0.8*H};
    Button menu_button = {{game_data->szeles-W-2*game_data->margo, game_data->magas-H-game_data->margo, W, H}, feher, fekete, "Menü", go_to_menu};
    Button settings_button = {{2*game_data->margo, game_data->magas-H-game_data->margo, W, H}, feher, fekete, "Beállítások", go_to_settings};
    Button buttons[2] = {menu_button, settings_button};
    char input[HOSSZ] = "";
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE] = "";
    char textandcomposition[HOSSZ + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1] = "";
    SDL_Rect* word_rects = calculate_Rects(text, font, game_data->margo, game_data->kocsi_h + 2*game_data->margo, game_data->szeles-2*game_data->margo);
    int i=0;
    bool draw = true;
    bool quit = false;
    bool countdown_over = false;
    int countdown = 5;
    SDL_TimerID id = SDL_AddTimer(1000, idozit, NULL);
    SDL_StartTextInput();
    SDL_Event event;
    clock_t t;
    while (!quit && game_data->game_view == SingleGame && i <text.word_count && SDL_WaitEvent(&event)) {
        char* target = text.words[i];
        switch (event.type) {
            case TICK_SEC:
                if (!countdown_over) {
                    draw = true;
                }
                break;
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
            clear_screen(game_data, vilagos_kek);
            bool inputCorrect = input_correct(target, input);
            SDL_Color inputColor = inputCorrect ? feher : vilagos_piros;
            if (!inputCorrect && correct[i]) {
                correct[i] = false;
            }
            render_input(input, input_box, inputColor , font, renderer, composition, textandcomposition);
            render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
            render_car(renderer, sotet_piros, fekete, game_data->margo+(game_data->szeles-game_data->kocsi_w-game_data->margo)*i/text.word_count, game_data->margo, game_data->kocsi_w, game_data->kocsi_h);
            for (int i=0; i<num_buttons; i++) {
                render_button(buttons[i], renderer, font);
            }
            if (!countdown_over) {
                handle_countdown_s(&countdown_over, countdown_display, &countdown, font, &t, renderer);
            }
            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
    SDL_StopTextInput();
    game_data->stats = calculate_stats(text.word_count, correct, times);
    free(correct);
    free(times);
    free(word_rects);
    SDL_RemoveTimer(id);
    if (game_data->game_view == SingleGame) {
        game_data->game_view = Statistics;
        event.type = GAME_VIEW_CHANGED_EVENT;
        SDL_PushEvent(&event);
    }
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
    Button settings_button = {{game_data->szeles/2-W/2, 90, W, H}, feher, fekete, "Beállítások & Ranglista", go_to_settings};
    Button single_game_button = {{game_data->szeles/2-W/2, settings_button.rect.y+H+30, W, H}, feher, fekete, "Játék Egyedül", go_to_game};
    Button bot_game_button = {{game_data->szeles/2-W/2, single_game_button.rect.y+H+30, W, H}, feher, fekete, "Játék Botok Ellen", go_to_game};
    Button multi_game_button = {{game_data->szeles/2-W/2, bot_game_button.rect.y+H+30, W, H}, feher, fekete, "Többszemélyes Játék", go_to_game};
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

void render_settings(GameData* game_data, SDL_Color color1, SDL_Color color2) {
    int top = (int)(game_data->margo);
    int bottom = (int)(game_data->magas - game_data->margo);
    int left = (int)(game_data->margo);
    int right = (int)(game_data->szeles*0.4);
    TTF_Font* font = game_data->font;
    SDL_Renderer* renderer = game_data->renderer;
    boxRGBA(renderer, left, top, right, bottom, color2.r, color2.g, color2.b, 255);
    rectangleRGBA(renderer, left, top, right, bottom, color1.r, color1.g, color1.b, 255);
    SDL_Rect rect = render_string_blended("Beállítások:", color1, font, left, top, renderer, TopLeft);
    hlineRGBA(renderer, left, right, game_data->margo+60, color1.r, color1.g, color1.b, 255);
    top += rect.h + game_data->margo;
    int h = (bottom-top) / LEADERBOARD_SIZE; //esztétkailag ez a szép, habár nincs köze a ranglistához
    for (int i=0; i<4; i++) {
        char display_str[2*HOSSZ];
        sprintf(display_str, "%s WPM: %.0f", game_data->bots[i].name, game_data->bots[i].expected_wpm);
        render_string_blended(display_str, color1, font, (left+right)/2, top+h*(i+0.5), renderer, Middle);
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
    Button reset_leader_button = {{game_data->szeles - game_data->margo-160, game_data->margo, 160, 60}, feher, fekete, "Törlés", reset_leaderboard};
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
    sprintf(display_str, "WPM: %.2f. Pontosság: %.2f%%.", stats.wpm, stats.accuracy*100);
    Button menu_button = {{game_data->szeles/2-100, game_data->magas-150, 200, 100}, feher, fekete, "Menu", go_to_menu};
    Button settings_button = {{menu_button.rect.x, menu_button.rect.y - menu_button.rect.h-game_data->margo, menu_button.rect.w, menu_button.rect.h}, feher, fekete, "Settings", go_to_settings};
    if (top10(leaderboard, stats.wpm)) {
        settings_button.str = "Rögzítés";
        settings_button.func = get_name;
        strcat(display_str, " Ez ranglistás eredmény!");
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
    sprintf(display_str, "WPM: %.2f, adja meg a nevét hogy rögzíthessük!", game_data->stats.wpm);
    render_string_blended(display_str, fekete, font, game_data->szeles/2, game_data->magas/2-2*game_data->margo, renderer, Middle);
    SDL_RenderPresent(renderer);
    SDL_Rect input_box = {game_data->margo, game_data->magas/2, game_data->szeles-2*game_data->margo, game_data->margo};
    char input[HOSSZ];
    input_text(input, HOSSZ, input_box, feher, fekete, font, renderer);
    printf("Got this: %s\n", input);
    LeaderboardEntry entry;
    entry.wpm = game_data->stats.wpm;
    strcpy(entry.name, input);
    update_leaderboard(&(game_data->leaderboard), entry);
    print_leaderboard(game_data->leaderboard);
    game_data->game_view = Settings;
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
}

void init_bots(Bot bots[]) {
    for (int i=0; i<4; i++) {
        bots[i].expected_wpm = 30 + i*5;
        bots[i].spread_ms = (i+1)*600;
        sprintf(bots[i].name, "BOT%d", i+1);
        bots[i].ms = (int)(60.0*1000.0/bots[i].expected_wpm);
        bots[i].ms += (rand()%bots[i].spread_ms) - bots[i].spread_ms/2;
        printf("%s ms: %d\n", bots[i].name, bots[i].ms);
    }

}

int main(int argc, char *argv[]) {
    srand(time(0)); //inicializáljuk a randomszám generátort
    const int magas = 600;
    const int szeles = 1000;
    const int margo = 40;
    const int kocsi_w = szeles/8;
    const int kocsi_h = kocsi_w/2;
    Bot bots[4];
    init_bots(bots);
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    TTF_Font *title;
    Stats stats = {0, 0};
    sdl_init(szeles, magas, FONT, &window, &renderer, &font, &underlined, &title);
    TextArray textarray = parse_file("hobbit_short.txt");
    LeaderBoard leaderboard;
    if (!load_leaderboard(&leaderboard)) {
        printf("Could not load leaderboard.\n");
    } else {
        print_leaderboard(leaderboard);
    }
    GameView game_view = MainMenu;
    GameData game_data = {stats, leaderboard, renderer, font, underlined, title, &textarray, game_view, magas, szeles, margo, kocsi_w, kocsi_h, bots};
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
                    case Statistics:
                        statistics(&game_data);
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
    save_leaderboard(game_data.leaderboard);
    free_textarray(&textarray);
    sdl_close(&window, &renderer, &font, &underlined, &title);
    return 0;
}
