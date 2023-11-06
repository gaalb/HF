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

/*We will need some TextArray->str convesrion*/
//char szoveg[] = "Gandalf! If you had heard only a quarter of what I have heard about him, and I have only heard very little of all there is to hear, you would be prepared for any sort I of remarkable tale. Tales and adventures sprouted up all over the place wherever he went, in the most extraordinary fashion. He had not been down that way under The Hill for ages and ages, not since his friend the Old Took died, in fact, and the hobbits had almost forgotten what he looked like.";

int main(int argc, char *argv[]) {
    srand(time(0));
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    sdl_init(SZELES, MAGAS, FONT ,&window, &renderer, &font);

    TextArray TheLordOfTheRings = parse_file("lotr_chunks.txt");
    /*Do whatever we want:*/
    Text text = TheLordOfTheRings.texts[rand()%TheLordOfTheRings.text_count];
    print_text(text);
    char* szoveg = text_to_string(text);


    SDL_Color feher = {255, 255, 255};

    /* hatter */
    for (int i = 0; i < 20000; ++i)
        filledCircleRGBA(renderer, rand() % SZELES, rand() % MAGAS,
                         10 + rand() % 5, rand() % 256, rand() % 256, rand() % 256, 64);



    /* felirat megrajzolasa, kulonfele verziokban */
    SDL_Surface *felirat;
    SDL_Texture *felirat_t;
    SDL_Rect hova = { 0, 0, 0, 0 };
    /* ha sajat kodban hasznalod, csinalj belole fuggvenyt! */
    felirat = TTF_RenderUTF8_Blended_Wrapped(font, szoveg, feher,SZELES-20);
    felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
    hova.x = 20;
    hova.y = 20;
    hova.w = felirat->w;
    hova.h = felirat->h;
    SDL_RenderCopy(renderer, felirat_t, NULL, &hova);
    SDL_FreeSurface(felirat);
    SDL_DestroyTexture(felirat_t);



    SDL_RenderPresent(renderer);

    /* nem kell tobbe */
    TTF_CloseFont(font);

    SDL_Event event;
    while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
    }

    SDL_Quit();

    /*free everything*/
    free(szoveg);
    free_textarray(&TheLordOfTheRings);
    return 0;
}

