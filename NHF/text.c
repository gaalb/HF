#include "text.h"

/*
Kirakja standard kimenetre a Text objektumot: csak bebugoláshoz
Text text: a szóban forgó szöveg
*/
void print_text(Text text) {
    for (int i=0; i<text.word_count; i++) {
        printf("%s ", text.words[i]);
    }
    printf("\n");
}

/*
Felszabadít egy Text objektumot, melynek words tömbjében
minden elem egy-egy szintén dinamikusan foglalt sztring,
ezért a Text felszabadítása az egyes sztringek
felszabadítása, a pointer és a szó szám kinullázása
Text* text: a felszabadítandó Text pointere
*/
void free_text(Text* text) {
    for (int i=0; i<text->word_count; i++) {
        free(text->words[i]);
    }
    free(text->words);
    text->word_count = 0;
    text->words = NULL;
}

/*
Felszabadít egy TextArray-t, amely Text-ek dinamikus tömbje:
ezeket egyenként fel kell szabadítani
Textarray* textarray: a felszabadítandó struktúra
*/
void free_textarray(TextArray* textarray) {
    for (int i=0; i<textarray->text_count; i++) {
        free_text(&(textarray->texts[i]));
    }
    free(textarray->texts);
    textarray->text_count = 0;
    textarray->texts = NULL;
}

/*
A read_word_from_file segédfüggvénye, amellyel rekurzívan lehet
fájlból olvasni egy stringet, úgy, hogy a stack-et használjuk
átmeneti tárolóként, így egyetlen malloc hívásra van csak szükség
int n: a jelenleg stacken tárolt karakterek száma, első hívásnál nyilván 0
File* file: a fájl, amiből olvasunk
*/
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

/*
a recursive_read fg-vt használva rekurzív módon addig
olvas karaktereket, amíg egy szóhatárhoz nem jut, majd ezeket
a karaktereket egy dinamikusan foglalt stringbe teszi és visszatér
a dinamikusan foglalt memória pointerével, melyet később fel
kell szabadítani
FIle* file: a fájl, amiből olvasunk
*/
char* read_word_from_file(FILE* file) {
    return recursive_read(0, file);
}

/*
feltéve, hogy a megfelelő formátumban van a fájl, beolvas
belőle egy Text struktúrát, amely egy dinamikus tömb, vagyis
majd később fel kell szabadítani
egy Text felépítése:
WordCount: <SZÁM>
TextTextText ... \n
Vagyis előre megmondja milyen hosszú. Lehetne a \n-t is végjelként
kezelni, de így átláthatóbb a foglalás.
File* file: a fájl, amiből olvasunk
*/
Text read_text_from_file(FILE* file) {
    int word_count, read;
    Text text = {0, NULL};
    //előbb a szavak számát kérjük be, a foglaláshoz
    read = fscanf(file, "\nWordCount: %d\n", &word_count);
    if (read != 1) {
        printf("Couldn't read word count.");
        return text;
    }
    //majd helyet foglalunk
    text.word_count = word_count;
    text.words = (char**) malloc(sizeof(char*)*word_count);
    if (text.words == NULL) {
        printf("Couldn't allocate memory for a piece of text.");
        free_text(&text);
        return text;
    }
    //és végül feltöltjük stringekkel a words tömböt
    for (int j=0; j<word_count; j++) {
        char* word = read_word_from_file(file);
        text.words[j] = word;
    }
    return text;
}

/*
Végigpásztázza a megadott fájlt, és a beolvasható Text struktúrákat
berakja egy dinamikusan foglalt tömbbe.
Ez azt is jelenti, hogy az általa visszaadott TextArray-t fel is kell
szabadítani!
char filename[]: a szóban forgó fájl neve
*/
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
