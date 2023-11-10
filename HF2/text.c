#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "debugmalloc.h"
#include "text.h"

//standard outputra rakja ki a textet
void print_text(Text text) {
    for (int i=0; i<text.word_count; i++) {
        printf("%s ", text.words[i]);
    }
    printf("\n");
}

/*egy Text.words tömb minden eleme egy-egy szintén dinamikusan
foglalt sztring, ezért a Text felszabadítása az egyes sztringek
felszabadítása, a pointer és a szó szám kinullázása*/
void free_text(Text* text) {
    for (int i=0; i<text->word_count; i++) {
        free(text->words[i]);
    }
    free(text->words);
    text->word_count = 0;
    text->words = NULL;
}

/*a TextArray Text-ek dinamikus tömbje: fel kell szabadítani õket majd
kinullázni a pointert és a számlálót*/
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

/*a felette lévõ függvénnyt használva rekurzív módon addig
olvas karaktereket, amíg egy szóhatárhoz nem jut, majd ezeket
a karaktereket egy dinamikusan foglalt stringbe teszi és visszatér
a dinamikusan foglalt memória pointerével, melyet késõbb fel
kell szabadítani*/
char* read_word_from_file(FILE* file) {
    return recursive_read(0, file);
}

/*feltéve, hogy a megfelelõ formátumban van a fájl, beolvas
belõle egy Text struktúrát, amely egy dinamikus tömb, vagyis
majd késõbb fel kell szabadítani
egy Text az alábbi szerint néz ki:
WordCount: <SZÁM>
TextTextText ... \n
Vagyis elõre megmondja milyen hosszú. Lehetne a \n-t is végjelként
kezelni, de így átláthatóbb a foglalás.*/
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

/*Végigpásztázza a megadott fájlt, és a beolvasható Text struktúrákat
bekraja egy dinamikusan foglalt tömbbe.
Ez azt is jelenti, hogy az általa visszaadott TextArray-t fel is kell
szabadítani!*/
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
