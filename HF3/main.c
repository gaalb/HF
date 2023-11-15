#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "debugmalloc.h"
#include "text.h"

#define HOSSZ 100
#define SZELES 1000
#define MAGAS 600
#define FONT "LiberationSerif-Regular.ttf"
#define NEXT_WORD_EVENT SDL_USEREVENT+1

/*A programozás alapjai 1 házi feladat: Typeracer
Gaál Botond
CRQEYD
2023.11.12.*/

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

/*megadja, két string közül az első hány karater egyezik meg*/
int match_len(char* target, char* input) {
    int l=0;
    while (target[l] != '\0' && input[l] != '\0') {
        if (target[l] == input[l]) {
            l++;
        } else {
            break;
        }
    }
    return l;
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

/*Megadja, milyen színűre kell kiszinezni a beviteli szövegdobozt: ha minden
begépelt karakter helyes, akkor fehérre, ellenkező esetben pirosra*/
SDL_Color input_color(char* target, char* input, SDL_Color color1, SDL_Color color2) {
    int input_len = strlen(input);
    if (match_len(target, input) == input_len) {
        return color1;
    } else {
        return color2;
    }
}

void render_input(char *input, SDL_Rect teglalap, SDL_Color input_color, TTF_Font *font, SDL_Renderer *renderer, char* composition, char* textandcomposition) {
    SDL_Color fekete = {0, 0, 0}; //a border color kb. mindig fekete
    /* Max hasznalhato szelesseg */
    int maxw = teglalap.w - 2;
    int maxh = teglalap.h - 2;
    /* doboz kirajzolasa */
    boxRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, input_color.r, input_color.g, input_color.b, 255);
    rectangleRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, fekete.r, fekete.g, fekete.b, 255);
    /* szoveg kirajzolasa */
    int w;
    strcpy(textandcomposition, input);
    strcat(textandcomposition, composition);
    if (textandcomposition[0] != '\0') {
        SDL_Surface *felirat = TTF_RenderUTF8_Blended(font, textandcomposition, fekete);
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
        vlineRGBA(renderer, teglalap.x + w + 2, teglalap.y + 2, teglalap.y + teglalap.h - 3, fekete.r, fekete.g, fekete.b, 192);
    }
}

void render_string_to_rect_blended(char* str, SDL_Color color, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer) {
    SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, str, color);
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
}

void render_string_to_rect_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer) {
    SDL_Surface* word_s  = TTF_RenderUTF8_Shaded(font, str, color, background);
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
}

SDL_Rect render_string_blended(char* str, SDL_Color color, TTF_Font* font, int x, int y, SDL_Renderer* renderer) {
    SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, str, color);
    SDL_Rect rect;
    rect.w = word_s->w;
    rect.h = word_s->h;
    rect.x = x;
    rect.y = y;
    x += rect.w;
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
    return rect;
}

SDL_Rect render_string_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, int x, int y, SDL_Renderer* renderer) {
    SDL_Surface* word_s  = TTF_RenderUTF8_Shaded(font, str, color, background);
    SDL_Rect rect;
    rect.w = word_s->w;
    rect.h = word_s->h;
    rect.x = x;
    rect.y = y;
    x += rect.w;
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
    return rect;
}

int get_cursor_index(Text text, int target_index, int input_len, int* cursor_len) {
    int target_len = strlen(text.words[target_index]);
    while (input_len > target_len+1 && target_index < text.word_count-1) {
        target_index++;
        target_len += strlen(text.words[target_index])+1;
    }
    *cursor_len = input_len - target_len + strlen(text.words[target_index]);
    return target_index;
}

SDL_Rect* calculate_Rects(Text text, TTF_Font* font, int x, int y, int w, SDL_Color color) {
    int word_count = text.word_count;
    SDL_Rect* word_rects = (SDL_Rect*) malloc(word_count*sizeof(SDL_Rect));
    int right_edge = x+w;
    int left_edge = x;
    for (int i=0; i<word_count; i++) {
        char word[HOSSZ];
        strcpy(word, text.words[i]);
        strcat(word, " ");
        SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, word, color);
        word_rects[i].w = word_s->w;
        word_rects[i].h = word_s->h;
        if (x+word_s->w > right_edge) {
            y += word_s -> h;
            x = left_edge;
        }
        word_rects[i].x = x;
        word_rects[i].y = y;
        x += word_s->w;
        SDL_FreeSurface(word_s);
    }
    return word_rects;
}

void clear_screen(SDL_Renderer* renderer, SDL_Color color) {
    boxRGBA(renderer, 0, 0, SZELES, MAGAS, color.r, color.g, color.b, 255);
}

void render_cursor(Text text, SDL_Rect* word_rects, TTF_Font* font, int cursor_index, int cusor_len, SDL_Renderer* renderer) {
    SDL_Color fekete = {0, 0, 0};
    SDL_Rect rect = word_rects[cursor_index];
    char display_str[HOSSZ];
    char *word = text.words[cursor_index];
    int x1, x2, y1, y2;
    if (cusor_len) {
        strcpy(display_str, word);
        strcat(display_str, " ");
        display_str[cusor_len] = '\0'; //így nem kell hibakezelni, automatikusan kibővül egy space-el, ha szükséges
        SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, display_str, fekete);
        x1 = rect.x +  (word_s->w) - 1;
    } else {
        x1 = rect.x - 1;
    }
    x2 = x1 + 2;
    y1 = rect.y;
    y2 = y1 + rect.h;
    boxRGBA(renderer, x1, y1, x2, y2, fekete.r, fekete.g, fekete.b, 255);
}

void render_Text(Text text, TTF_Font* font, TTF_Font* underlined, SDL_Rect* word_rects, SDL_Renderer* renderer, SDL_Color color1, SDL_Color color2, SDL_Color color3, int target_index, char* input) {
    char* word;
    char* target = text.words[target_index];
    char display_str[HOSSZ];
    int input_len = strlen(input);
    int target_len = strlen(target);
    int cursor_len = 0;
    int cursor_index = get_cursor_index(text, target_index, input_len, &cursor_len);
    render_cursor(text, word_rects, font, cursor_index, cursor_len, renderer);
    SDL_Rect rect;
    int i;
    for (i=0; i<target_index; i++) {
        //a jól begépelt szavakat zölddel írjuk ki
        word = text.words[i];
        rect = word_rects[i];
        strcpy(display_str, word);
        strcat(display_str, " ");
        render_string_to_rect_blended(display_str, color2, font, rect, renderer);
    }
    /*ez az eset a legbonyolultabb:
    -az aktív szót alá kell húzni
    -a jó inputokat zölddel kell jelezni
    -a rossz inputokat piros háttérrel kell jelezni
    -a még nem begépelt szöveget feketével kell jelezni
    -lehet csak piros is, csak zöld is, csak fekete is, sőt ezeknek kombinációi
    -a piros és fekete szavak határa lehet az aláhúzott részen is, de lehet a nem aláhúzott részen is*/
    int x = word_rects[i].x;
    int y = word_rects[i].y;
    int green_target_len = match_len(target, input);
    int red_target_len = input_len <= target_len? input_len - green_target_len : target_len - green_target_len;
    int red_len = input_len - green_target_len - red_target_len;
    int black_target_len = target_len - green_target_len - red_target_len;
    if (green_target_len) {
        strcpy(display_str, target);
        display_str[green_target_len] = '\0';
        rect = render_string_blended(display_str, color2, underlined, x, y, renderer);
        x += rect.w;
    }
    /*ezen a ponton kiírtuk a helyesen beírt részét a beírandó szónak:
    ezután addig kell pirossal írnunk a dolgokat, amíg van piros, majd
    ha maradt még valami a kurzoros szóból, akkor annak ki kell írni a
    maradékját feketével*/
    if (red_target_len) {
        strcpy(display_str, target+green_target_len);
        display_str[red_target_len] = '\0';
        rect = render_string_shaded(display_str, color1, color3, underlined, x, y, renderer);
        x += rect.w;
    }
    if (black_target_len) {
        strcpy(display_str, target+green_target_len+red_target_len);
        rect = render_string_blended(display_str, color1, underlined, x, y, renderer);
        x += rect.w;
    }
    if (red_len) { //adjunk hozzá egy space-t
        rect = word_rects[i];
        red_len--;
        SDL_Rect space_rect;
        space_rect.x = x;
        space_rect.y = y;
        space_rect.w = rect.x + rect.w - x;
        space_rect.h = rect.h;
        render_string_to_rect_shaded(" ", color1, color3, font, space_rect, renderer);
    }
    if (i < text.word_count-1) {
        i++;
    } else {
        red_len = 0;
    }
    while (red_len) {
        rect = word_rects[i];
        word = text.words[i];
        int word_len = strlen(word);
        if (red_len > word_len) { //vagyis nem lesz benne fekete
            strcpy(display_str, word);
            strcat(display_str, " ");
            render_string_to_rect_shaded(display_str, color1, color3, font, rect, renderer);
            red_len -= (word_len + 1);
        } else {
            strcpy(display_str, word);
            display_str[red_len] ='\0';
            x = rect.x;
            y = rect.y;
            rect = render_string_shaded(display_str, color1, color3, font, x, y, renderer);
            x += rect.w;
            strcpy(display_str, word+red_len);
            strcat(display_str, " ");
            render_string_blended(display_str, color1, font, x, y, renderer);
            x += rect.w;
            red_len = 0;
        }
        i++;
    }
    for (i=cursor_index+1; i<text.word_count; i++) {
        //a még nem begépelt szavakat feketével írjuk ki
        word = text.words[i];
        rect = word_rects[i];
        strcpy(display_str, word);
        strcat(display_str, " ");
        render_string_to_rect_blended(display_str, color1, font, rect, renderer);
    }

}

int main(int argc, char *argv[]) {
    srand(time(0)); //inicializáljuk a randomszám generátort
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    sdl_init(SZELES, MAGAS, FONT ,&window, &renderer, &font, &underlined);
    /*A felhasznált színek:*/
    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color zold = {0, 255, 0};
    SDL_Color vilagos_kek = {120, 150, 255};
    SDL_Color vilagos_piros = {255, 114, 118};
    //SDL_Color sotet_piros = {112, 57, 63};
    TextArray TheLordOfTheRings = parse_file("hobbit_short.txt");
    Text text = TheLordOfTheRings.texts[rand()%TheLordOfTheRings.text_count];
    //Text text = TheLordOfTheRings.texts[30];
    SDL_Rect input_box = {20, MAGAS/2, SZELES-40, 40};
    char input[HOSSZ] = "";
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    char textandcomposition[HOSSZ + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    SDL_Rect* word_rects = calculate_Rects(text, font, 20, 20, SZELES-30, fekete);
    int i=0;
    bool quit = false;
    SDL_StartTextInput();
    SDL_Event event;
    while (SDL_WaitEvent(&event) && !quit && i <text.word_count) {
        char* target = text.words[i];
        switch (event.type) {
            /* Kulonleges karakter */
             case SDL_KEYDOWN:
                 if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    handle_backspace(input);
                    clear_screen(renderer, vilagos_kek);
                    render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
                    render_input(input, input_box, input_color(target, input, feher, vilagos_piros), font, renderer, composition, textandcomposition);
                    SDL_RenderPresent(renderer);
                 }
                 if (event.key.keysym.sym == SDLK_SPACE) {
                    handle_space(input, target, composition, event);
                    clear_screen(renderer, vilagos_kek);
                    render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
                    render_input(input, input_box, input_color(target, input, feher, vilagos_piros), font, renderer, composition, textandcomposition);
                    SDL_RenderPresent(renderer);
                 }
                 break;
            case SDL_TEXTINPUT:
                /* A feldolgozott szoveg bemenete */
                handle_textinput(input, composition, event);
                clear_screen(renderer, vilagos_kek);
                render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
                render_input(input, input_box, input_color(target, input, feher, vilagos_piros), font, renderer, composition, textandcomposition);
                SDL_RenderPresent(renderer);
                break;
            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                strcpy(composition, event.edit.text);
                clear_screen(renderer, vilagos_kek);
                render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
                render_input(input, input_box, input_color(target, input, feher, vilagos_piros), font, renderer, composition, textandcomposition);
                SDL_RenderPresent(renderer);
                break;
            case NEXT_WORD_EVENT:
                i++;
                if (i == text.word_count) {
                    break;
                }
                SDL_StartTextInput();
                input[0] = '\0';
                composition[0] = '\0';
                clear_screen(renderer, vilagos_kek);
                render_Text(text, font, underlined, word_rects, renderer, fekete, zold, vilagos_piros, i, input);
                render_input(input, input_box, input_color(target, input, feher, vilagos_piros), font, renderer, composition, textandcomposition);
                SDL_RenderPresent(renderer);
                break;
            case SDL_QUIT:
                /* visszatesszuk a sorba ezt az eventet, mert
                 * sok mindent nem tudunk vele kezdeni */
                SDL_PushEvent(&event);
                quit = true;
                break;
        }
    }
    SDL_StopTextInput();
    free_textarray(&TheLordOfTheRings);
    free(word_rects);
    sdl_close(&window, &renderer, &font);
    return 0;
}
