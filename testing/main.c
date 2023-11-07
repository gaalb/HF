#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "debugmalloc.h"

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

int main()
{
    srand(time(0));
    #if 0
    TextArray TheLordOfTheRings = parse_file("lotr_chunks.txt");
    /*Do whatever we want:*/
    Text text = TheLordOfTheRings.texts[rand()%TheLordOfTheRings.text_count];
    print_text(text);
    char* str = text_to_string(text);
    printf("%s\n", str);
    /*free everything*/
    free(str);
    free_textarray(&TheLordOfTheRings);
    #endif




    return 0;
}
