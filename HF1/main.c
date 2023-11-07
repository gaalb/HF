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
#define FONT "LiberationSerif-Regular.ttf"
#define HOSSZ 100

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

/*This function reads a word from the provided file, dynamically
allocates space for it in memory, copies the word to the place in
memory and returns the pointer. Note that it is the responsibility
of the caller to free that memory.*/
char* read_word_from_file(FILE* file) {
    int len=0;
    char *word = (char*) malloc(sizeof(char)*1);
    if (word == NULL) {
        return NULL;
    }
    word[0] = '\0';
    char c;
    /*Read characters until we hit a newline or a space, and
    after each character, make a longer array to store the word in.*/
    while (fscanf(file, "%c", &c) == 1 && c != ' ' && c != '\n') {
        char *newword = (char*) malloc(sizeof(char) * (len+2));
        if (newword == NULL) {
            free(word);
            return NULL;
        }
        strcpy(newword, word);
        free(word);
        word = newword;
        word[len] = c;
        word[len+1] = '\0';
        len++;
    }
    return word;
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
    TextArray TheLordOfTheRings;
    int text_count;
    if (file != NULL && fscanf(file, "TextCount: %d", &text_count) == 1) {
        TheLordOfTheRings.text_count = text_count;
        TheLordOfTheRings.texts = (Text*) malloc(sizeof(Text)*text_count);
        if (TheLordOfTheRings.texts == NULL) {
            printf("Couldn't allocate memory for TheLordOfTheRings");
            TheLordOfTheRings.text_count = 0;
        } else {
            for (int i=0; i<text_count; i++) {
                TheLordOfTheRings.texts[i] = read_text_from_file(file);
            }
        }
    } else {
        perror("Couldn't open file, or file was wrong format.");
    }
    fclose(file);
    return TheLordOfTheRings;
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

void sdl_init(int szeles, int magas, const char* tipus, SDL_Window** pwindow, SDL_Renderer** prenderer, TTF_Font** pfont) {
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
    *pwindow = window;
    *prenderer = renderer;
    *pfont = font;
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

void render_text(char* text, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer, int x, int y) {
    SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font, text, color);
    SDL_Texture* text_t = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = text_surface->w;
    rect.h = text_surface->h;
    SDL_RenderCopy(renderer, text_t, NULL, &rect);
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_t);
}

SDL_Rect render_text_wrapped(char* text, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer, int x, int y, int w) {
    SDL_Surface* text_surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, w);
    SDL_Texture* text_t = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = text_surface->w;
    rect.h = text_surface->h;
    SDL_RenderCopy(renderer, text_t, NULL, &rect);
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_t);
    return rect;
}

bool input_target_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, TTF_Font *font, SDL_Renderer *renderer, char* target) {
    printf("%s\n", target);
    /* Ez tartalmazza az aktualis szerkesztest */
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    composition[0] = '\0';
    /* Ezt a kirajzolas kozben hasznaljuk */
    char textandcomposition[hossz + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    /* Max hasznalhato szelesseg */
    int maxw = teglalap.w - 2;
    int maxh = teglalap.h - 2;

    dest[0] = '\0';

    bool next = false;
    bool kilep = false;

    SDL_StartTextInput();
    while (!kilep && !next) {
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
                if (event.key.keysym.sym == SDLK_SPACE) {
                    if (strcmp(dest, target)==0) {
                        next = true;
                    }
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
    return next;
}


int main(int argc, char *argv[]) {
    srand(time(0));
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    sdl_init(SZELES, MAGAS, FONT ,&window, &renderer, &font);
    TextArray TheLordOfTheRings = parse_file("hobbit_short.txt");
    /*Do whatever we want:*/
    Text text = TheLordOfTheRings.texts[rand()%TheLordOfTheRings.text_count];
    print_text(text);
    char* szoveg = text_to_string(text);


    SDL_Color fekete = {0, 0, 0};
    SDL_Color feher = {255, 255, 255};
    SDL_Color zold = {0, 255, 0};

    /* hatter */
    for (int i = 0; i < 20000; ++i)
        filledCircleRGBA(renderer, rand() % SZELES, rand() % MAGAS,
                         10 + rand() % 5, rand() % 256, rand() % 256, rand() % 256, 64);


    char zold_szoveg[100];
    strncpy(zold_szoveg, szoveg, 100);
    zold_szoveg[100] = '\0';
    render_text_wrapped(szoveg, font, fekete, renderer, 20, 20, SZELES-20);
    render_text_wrapped(zold_szoveg, font, zold, renderer, 20, 20, SZELES-20);
    //rectangleRGBA(renderer, rect.x, rect.y, rect.x+rect.w, rect.y+rect.h, 255, 255, 255, 255);

    SDL_Rect input_box = {20, MAGAS/2, SZELES-40, 40};
    char input[HOSSZ];
    for (int i=0; i<text.word_count; i++) {
        input_target_text(input, HOSSZ, input_box, feher, fekete, font, renderer, text.words[i]);
    }

    SDL_RenderPresent(renderer);
    SDL_Event event;
    while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
    }
    /*free everything*/
    free(szoveg);
    free_textarray(&TheLordOfTheRings);
    sdl_close(&window, &renderer, &font);
    return 0;
}

