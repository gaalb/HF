#include "text.h"

//standard outputra rakja ki a textet
void print_text(Text text) {
    for (int i=0; i<text.word_count; i++) {
        printf("%s ", text.words[i]);
    }
    printf("\n");
}

/*egy Text.words t�mb minden eleme egy-egy szint�n dinamikusan
foglalt sztring, ez�rt a Text felszabad�t�sa az egyes sztringek
felszabad�t�sa, a pointer �s a sz� sz�m kinull�z�sa*/
void free_text(Text* text) {
    for (int i=0; i<text->word_count; i++) {
        free(text->words[i]);
    }
    free(text->words);
    text->word_count = 0;
    text->words = NULL;
}

/*a TextArray Text-ek dinamikus t�mbje: fel kell szabad�tani �ket majd
kinull�zni a pointert �s a sz�ml�l�t*/
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

/*a felette l�v� f�ggv�nnyt haszn�lva rekurz�v m�don addig
olvas karaktereket, am�g egy sz�hat�rhoz nem jut, majd ezeket
a karaktereket egy dinamikusan foglalt stringbe teszi �s visszat�r
a dinamikusan foglalt mem�ria pointer�vel, melyet k�s�bb fel
kell szabad�tani*/
char* read_word_from_file(FILE* file) {
    return recursive_read(0, file);
}

/*felt�ve, hogy a megfelel� form�tumban van a f�jl, beolvas
bel�le egy Text strukt�r�t, amely egy dinamikus t�mb, vagyis
majd k�s�bb fel kell szabad�tani
egy Text az al�bbi szerint n�z ki:
WordCount: <SZ�M>
TextTextText ... \n
Vagyis el�re megmondja milyen hossz�. Lehetne a \n-t is v�gjelk�nt
kezelni, de �gy �tl�that�bb a foglal�s.*/
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

/*V�gigp�szt�zza a megadott f�jlt, �s a beolvashat� Text strukt�r�kat
bekraja egy dinamikusan foglalt t�mbbe.
Ez azt is jelenti, hogy az �ltala visszaadott TextArray-t fel is kell
szabad�tani!*/
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
