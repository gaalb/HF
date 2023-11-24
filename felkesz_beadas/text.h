#ifndef TEXT_H
#define TEXT_H
//sztringek dinamikus t�mbje
typedef struct Text {
    int word_count;
    char** words;
} Text;

//text-ek dinamikus t�mbje
typedef struct TextArray {
    int text_count;
    Text* texts;
} TextArray;

void print_text(Text text);

void free_text(Text* text);

void free_textarray(TextArray* textarray);

char* recursive_read(int n, FILE* file);

char* read_word_from_file(FILE* file);

Text read_text_from_file(FILE* file);

TextArray parse_file(char filename[]);

#endif // TEXT_H
