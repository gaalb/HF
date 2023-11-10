#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "debugmalloc.h"

#define SZELES 1000
#define MAGAS 600
#define FONT "../arial.ttf"
#define HOSSZ 100
#define NEXT_WORD SDL_USEREVENT+1
#define CORRECT_EVENT SDL_USEREVENT+2
#define WRONG_EVENT SDL_USEREVENT+3

SDL_Color fekete = {0, 0, 0};
SDL_Color feher = {255, 255, 255};
SDL_Color zold = {0, 255, 0};
SDL_Color vilagos_kek = {120, 150, 255};
SDL_Color vilagos_piros = {255, 114, 118};
SDL_Color sotet_piros = {112, 57, 63};

typedef struct Text {
    int word_count;
    char** words;
} Text;

typedef struct TextArray {
    int text_count;
    Text* texts;
} TextArray;

void print_text(Text text) {
    for (int i=0; i<text.word_count; i++) {
        printf("%s ", text.words[i]);
    }
    printf("\n");
}

void free_text(Text* text) {
    for (int i=0; i<text->word_count; i++) {
        free(text->words[i]);
    }
    free(text->words);
    text->word_count = 0;
    text->words = NULL;
}

void free_textarray(TextArray* textarray) {
    for (int i=0; i<textarray->text_count; i++) {
        free_text(&(textarray->texts[i]));
    }
    free(textarray->texts);
    textarray->text_count = 0;
    textarray->texts = NULL;
}

char* recursive_read(int n, FILE* file) {
    char c;
    char* str;
    if (fscanf(file, "%c", &c) == 1 && c != ' ' && c != '\n') {
        str = recursive_read(n+1, file);
        str[n] = c;
    } else {
        str = (char*) malloc(sizeof(char) *(n+1));
        str[n] = '\0';
    }
    return str;
}

char* read_word_from_file(FILE* file) {
    return recursive_read(0, file);
}

/*This function reads a piece of text from the provided file,
and returns a Text-type variable, which has an array of strings
for which space is dynamically allocated. It is the caller's
responsibility to later free the space pointed by text.words.*/
Text read_text_from_file(FILE* file) {
    int word_count, read;
    Text text = {0, NULL};
    read = fscanf(file, "\nWordCount: %d\n", &word_count);
    if (read != 1) {
        printf("Couldn't read word count.");
        return text;
    }
    text.word_count = word_count;
    text.words = (char**) malloc(sizeof(char*)*word_count);
    if (text.words == NULL) {
        printf("Couldn't allocate memory for a piece of text.");
        free_text(&text);
        return text;
    }
    for (int j=0; j<word_count; j++) {
        char* word = read_word_from_file(file);
        text.words[j] = word;
    }
    return text;
}

/*This function generates a TextArray object, which is
a kind of dynamic array. To free it and every other dynamically
allocated member it has (texts), use free_textarray*/
TextArray parse_file(char filename[]) {
    FILE* file;
    file = fopen(filename, "r");
    TextArray textarray;
    int text_count;
    if (file != NULL && fscanf(file, "TextCount: %d", &text_count) == 1) {
        textarray.text_count = text_count;
        textarray.texts = (Text*) malloc(sizeof(Text)*text_count);
        if (textarray.texts == NULL) {
            printf("Couldn't allocate memory for TextArray");
            textarray.text_count = 0;
            free_textarray(&textarray);
        } else {
            for (int i=0; i<text_count; i++) {
                textarray.texts[i] = read_text_from_file(file);
            }
        }
    } else {
        perror("Couldn't open file, or file was wrong format.");
    }
    fclose(file);
    return textarray;
}


/*TODO: this sucks?*/
char* text_to_string(const Text text) {
    int len = 0;
    for (int i=0; i<text.word_count; i++) {
        len += strlen(text.words[i])+1;
    }
    char* str = (char*) malloc(sizeof(char)*len);
    str[0] = '\0';
    for (int i=0; i<text.word_count-1; i++) {
        strcat(str, text.words[i]);
        strcat(str, " ");
    }
    strcat(str, text.words[text.word_count-1]);
    return str;
}

void sdl_init(int szeles, int magas, const char* tipus, SDL_Window** pwindow, SDL_Renderer** prenderer, TTF_Font** pfont, TTF_Font** punderlined) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow("SDL peldaprogram", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szeles, magas, 0);
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

void render_struct_Text(Text text, TTF_Font* font, TTF_Font* underlined, SDL_Renderer* renderer, int x, int y, int w, int target_index, char* input) {
    boxRGBA(renderer, 0, 0, SZELES, MAGAS, vilagos_kek.r, vilagos_kek.g, vilagos_kek.b, 255);
    int right_edge = x+w;
    int left_edge = x;
    SDL_Rect rect;
    int i=0;
    char* target;
    char display_str[HOSSZ];
    while (i<text.word_count) {
        target = text.words[i];
        int target_len = strlen(target);
        int input_len = strlen(input);
        SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, target, fekete);
        if (x+word_s->w > right_edge) {
            y += word_s -> h;
            x = left_edge;
        }
        SDL_FreeSurface(word_s);
        if (i<target_index) {
            /*Ha egy regebbi, mar jol kiirt szot renderelunk: a kiirando szo
            a szo + szokoz*/
            strcpy(display_str, target);
            display_str[target_len] = ' ';
            display_str[target_len+1] = '\0';
            rect = render_string_blended(display_str, zold, font, x, y, renderer);
            x += rect.w;
            i++;
        } else if (i == target_index) {
            int green_len = match_len(target, input);
            int red_len = green_len < target_len ? input_len - green_len : 0;
            int black_len = input_len < target_len? target_len - input_len : 0;
            if (green_len){
                strcpy(display_str, target); //a helyesen beirt karakterek
                display_str[green_len] = '\0';
                rect = render_string_blended(display_str, zold, underlined, x, y, renderer);
                x += rect.w;
                if (!red_len) {
                    vlineRGBA(renderer, x+2, y + 2, y + rect.h - 3, fekete.r, fekete.g, fekete.b, 192);
                }
            }
            if (red_len) { //a helytelenul beirt karakterek
                strcpy(display_str, target+green_len);
                display_str[red_len] = '\0';
                rect = render_string_shaded(display_str, fekete, vilagos_piros, underlined, x, y, renderer);
                x += rect.w;
            }
            if (black_len) { //a meg nem beirt karakterek
                strcpy(display_str, target+green_len+red_len);
                rect = render_string_blended(display_str, fekete, underlined, x, y, renderer);
                if (!red_len) {
                    vlineRGBA(renderer, x+2, y + 2, y + rect.h - 3, fekete.r, fekete.g, fekete.b, 192);
                }
                x += rect.w;
            } else {
            }
            strcpy(display_str, " ");
            if (input_len > target_len) {
                rect = render_string_shaded(display_str, fekete, vilagos_piros, font, x, y, renderer);
            } else {
                rect = render_string_blended(display_str, fekete, font, x, y, renderer);
            }
            x += rect.w;
            red_len =  input_len - target_len - 1 > 0 ? input_len - target_len - 1 : 0;
            i++;
            while (red_len) {
                target = text.words[i];
                target_len = strlen(target);
                word_s = TTF_RenderUTF8_Blended(font, target, fekete);
                if (x+word_s->w > right_edge) {
                    y += word_s -> h;
                    x = left_edge;
                }
                SDL_FreeSurface(word_s);
                black_len = red_len > target_len + 1 ? 0 : target_len + 1 - red_len;
                if (!black_len) {
                    strcpy(display_str, target);
                    strcat(display_str, " ");
                    rect = render_string_shaded(display_str, fekete, vilagos_piros, font, x, y, renderer);
                    x += rect.w;
                    red_len = red_len - target_len - 1;
                } else {
                    strcpy(display_str, target);
                    strcat(display_str, " ");
                    display_str[red_len] = '\0';
                    rect = render_string_shaded(display_str, fekete, vilagos_piros, font, x, y, renderer);
                    x += rect.w;
                    strcpy(display_str, target+red_len);
                    strcat(display_str, " ");
                    rect = render_string_blended(display_str, fekete, font, x, y, renderer);
                    x += rect.w;
                    red_len = 0;
                }
                i++;
            }

        } else {
            strcpy(display_str, target);
            strcat(display_str, " ");
            rect = render_string_blended(display_str, fekete, font, x, y, renderer);
            x += rect.w;
            i++;
        }

    }

}

void render_input(char *input, SDL_Rect teglalap, SDL_Color input_color, TTF_Font *font, SDL_Renderer *renderer, char* composition, char* textandcomposition) {
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
            next.type = NEXT_WORD;
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

SDL_Color input_color(char* target, char* input) {
    int input_len = strlen(input);
    if (match_len(target, input) == input_len) {
        return feher;
    } else {
        return vilagos_piros;
    }
}

int main(int argc, char *argv[]) {
    srand(time(0));
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    sdl_init(SZELES, MAGAS, FONT ,&window, &renderer, &font, &underlined);
    TextArray TheLordOfTheRings = parse_file("hobbit_short.txt");
    Text text = TheLordOfTheRings.texts[rand()%TheLordOfTheRings.text_count];
    print_text(text);
    char* szoveg = text_to_string(text);
    SDL_Rect input_box = {20, MAGAS/2, SZELES-40, 40};
    char input[HOSSZ] = "";
    /* Ez tartalmazza az aktualis szerkesztest */
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    /* Ezt a kirajzolas kozben hasznaljuk */
    char textandcomposition[HOSSZ + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    int i=0;
    SDL_RenderPresent(renderer);
    SDL_Event event;
    bool quit = false;
    SDL_StartTextInput();
    while (SDL_WaitEvent(&event) && !quit && i <text.word_count) {
        char* target = text.words[i];
        SDL_RenderPresent(renderer);
        switch (event.type) {
            /* Kulonleges karakter */
             case SDL_KEYDOWN:
                 if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    handle_backspace(input);
                    render_struct_Text(text, font, underlined, renderer, 20, 20, SZELES-30, i, input);
                    render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                 }
                 if (event.key.keysym.sym == SDLK_SPACE) {
                    handle_space(input, target, composition, event);
                    render_struct_Text(text, font, underlined, renderer, 20, 20, SZELES-30, i, input);
                    render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                 }
                 break;
            case SDL_TEXTINPUT:
                /* A feldolgozott szoveg bemenete */
                handle_textinput(input, composition, event);
                render_struct_Text(text, font, underlined, renderer, 20, 20, SZELES-30, i, input);
                render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                break;
            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                strcpy(composition, event.edit.text);
                render_struct_Text(text, font, underlined, renderer, 20, 20, SZELES-30, i, input);
                render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                break;
            case NEXT_WORD:
                i++;
                if (i == text.word_count) {
                    break;
                }
                SDL_StartTextInput();
                input[0] = '\0';
                composition[0] = '\0';
                render_struct_Text(text, font, underlined, renderer, 20, 20, SZELES-30, i, input);
                render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                break;
                /*
            case WRONG_EVENT:
                input_color = vilagos_piros;
                render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                break;
            case CORRECT_EVENT:
                input_color = feher;
                render_input(input, input_box, input_color(target, input), font, renderer, composition, textandcomposition);
                break;*/
            case SDL_QUIT:
                /* visszatesszuk a sorba ezt az eventet, mert
                 * sok mindent nem tudunk vele kezdeni */
                SDL_PushEvent(&event);
                quit = true;
                break;
        }

    }
    SDL_StopTextInput();
    /*free everything*/
    free(szoveg);
    free_textarray(&TheLordOfTheRings);
    sdl_close(&window, &renderer, &font);
    return 0;
}

