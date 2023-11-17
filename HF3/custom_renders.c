#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include "custom_renders.h"
#include "constants.h"
#include "text.h"

/*az s1 stringbe belemásolja az s2 stringet úgy, hogy konkatenál egy space-t*/
char* add_space(char* s1, char* s2) {
    strcpy(s1, s2);
    return strcat(s1, " ");
}

/*megadja, két string közül az elsõ hány karater egyezik meg*/
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

/*Megadja, milyen színûre kell kiszinezni a beviteli szövegdobozt: ha minden
begépelt karakter helyes, akkor fehérre, ellenkezõ esetben pirosra*/
SDL_Color input_color(char* target, char* input, SDL_Color color1, SDL_Color color2) {
    int input_len = strlen(input);
    if (match_len(target, input) == input_len) {
        return color1;
    } else {
        return color2;
    }
}

/*kirajzolja a szövegdobozt, ahova a bemenetet gépeljük, a benne lévõ szöveggel együtt*/
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

/*kirenderel egy stringet háttérszín nélkül egy megadott dobozba*/
void render_string_to_rect_blended(char* str, SDL_Color color, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer) {
    SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, str, color);
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
}

/*kirenderel egy stringet háttérszínnel egy megadott dobozba*/
void render_string_to_rect_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer) {
    SDL_Surface* word_s  = TTF_RenderUTF8_Shaded(font, str, color, background);
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
}

/*kirenderel egy sztringet háttérszín nélkül egy adott x, y pozícióra, és
visszaadja a dobozt, amibe a sztring került*/
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

/*kirenderel egy sztringet háttérszínnel egy adott x, y pozícióra, és
visszaadja a dobozt, amibe a sztring került*/
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

/*kiszámolja, a Text objektum melyik szavába esik éppen a kurzor*/
int get_cursor_index(Text text, int target_index, int input_len, int* cursor_len) {
    int target_len = strlen(text.words[target_index]);
    while (input_len > target_len+1 && target_index < text.word_count-1) {
        target_index++;
        target_len += strlen(text.words[target_index])+1;
    }
    *cursor_len = input_len - target_len + strlen(text.words[target_index]);
    return target_index;
}

/*kiszámolja, a szöveg szavai mely dobozokba kell, hogy essenek,
ahhoz, hogy a szöveg beférjen a megadott helyre
x: a szövegdoboz bal széle
y: a szövegdoboz teteje
w: a szövegdoboz szélessége
a szövegdoboz magassága abból dõl el, hány sorba fog kiférni a szöveg*/
SDL_Rect* calculate_Rects(Text text, TTF_Font* font, int x, int y, int w, SDL_Color color) {
    int word_count = text.word_count;
    SDL_Rect* word_rects = (SDL_Rect*) malloc(word_count*sizeof(SDL_Rect));
    int right_edge = x+w;
    int left_edge = x;
    for (int i=0; i<word_count; i++) {
        char word[HOSSZ];
        SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, add_space(word, text.words[i]), color);
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

/*Letörli a táblát :) */
void clear_screen(SDL_Renderer* renderer, SDL_Color color) {
    boxRGBA(renderer, 0, 0, SZELES, MAGAS, color.r, color.g, color.b, 255);
}

/*A kurzor index és a kurzor szavon belüli pozíciója alapján rendereli a kurzort (egy elnyújtott téglalap)*/
void render_cursor(Text text, SDL_Rect* word_rects, TTF_Font* font, int cursor_index, int cusor_len, SDL_Renderer* renderer) {
    SDL_Color fekete = {0, 0, 0};
    SDL_Rect rect = word_rects[cursor_index];
    char display_str[HOSSZ];
    char *word = text.words[cursor_index];
    int x1, x2, y1, y2;
    if (cusor_len) {
        /*beírjuk display_str-be a szót, és */
        add_space(display_str, word);
        display_str[cusor_len] = '\0';
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

/*kirajzolja a szöveget:
text: a szöveg
font: az aláhúzatlan szövegtípus
underlined: ugyanaz mint font, csak aláhúzva
rects: a téglalapok listája, ahova az egyes szavak lesznek berajzolva
renderer: a renderer
color1: azoknak a szavaknak a színe, amik a kurzor után vannak (fekete)
color2: azoknak a szavaknak a színe, amik már helyesen be lettek írva (zöld)
color3: a háttérszín amivel jelöljük a helytelen gépelést (piros)
target_index: annak a szónak az indexe, amit éppen be kell gépelni
input: a játékos által begépelt szó*/
void render_Text(Text text, TTF_Font* font, TTF_Font* underlined, SDL_Rect* word_rects, SDL_Renderer* renderer, SDL_Color color1, SDL_Color color2, SDL_Color color3, int target_index, char* input) {
    char* word; //ez fog mutatni a kirajzolandó szóra
    char* target = text.words[target_index]; //ez mutat arra a szóra, amit aktuálisan be kell gépelni
    char display_str[HOSSZ]; //ebben fogjuk tárolni amit ki akarunk íratni
    int input_len = strlen(input);
    int target_len = strlen(target);
    int cursor_len = 0; //azon a szavon belül, ahol a kurzor áll, a kurzor helyzete
    int cursor_index = get_cursor_index(text, target_index, input_len, &cursor_len); //megadja melyik szóban áll a kurzor, valamint beállítja cursor_len-t
    render_cursor(text, word_rects, font, cursor_index, cursor_len, renderer);
    SDL_Rect rect;
    int i;
    for (i=0; i<target_index; i++) {
        //a jól begépelt szavakat zölddel írjuk ki
        word = text.words[i];
        rect = word_rects[i];
        render_string_to_rect_blended(add_space(display_str, word), color2, font, rect, renderer);
    }
    /*a helyesen begépelt szavak után a következõk a teendõk:
    -az aktív szót alá kell húzni
    -a jó inputokat zölddel kell jelezni
    -a rossz inputokat piros háttérrel kell jelezni
    -a még nem begépelt szöveget feketével kell jelezni
    -lehet csak piros is, csak zöld is, csak fekete is, sõt ezeknek kombinációi
    -a piros és fekete szavak határa lehet az aláhúzott részen is, de lehet a nem aláhúzott részen is*/
    int x = word_rects[i].x; //a következõ leírandó szövegrész x koordinátája
    int y = word_rects[i].y; //a következõ leírandó szövegrész y koordinátája
    int green_target_len = match_len(target, input); //az aláhúzott szó zöld része
    int red_target_len = input_len <= target_len? input_len - green_target_len : target_len - green_target_len; //az aláhúzott szó piros része
    int red_len = input_len - green_target_len - red_target_len; //a NEM aláhúzott szó/szavak piros része
    int black_target_len = target_len - green_target_len - red_target_len; //az aláhúzott szó fekete része
    if (green_target_len) { //zölddel aláhúzott rész
        strcpy(display_str, target);
        display_str[green_target_len] = '\0';
        rect = render_string_blended(display_str, color2, underlined, x, y, renderer);
        x += rect.w;
    } //pirossal aláhúzott rész
    if (red_target_len) {
        strcpy(display_str, target+green_target_len);
        display_str[red_target_len] = '\0';
        rect = render_string_shaded(display_str, color1, color3, underlined, x, y, renderer);
        x += rect.w;
    } //feketével aláhúzott rész
    if (black_target_len) {
        strcpy(display_str, target+green_target_len+red_target_len);
        rect = render_string_blended(display_str, color1, underlined, x, y, renderer);
        x += rect.w;
    } //ha volt még piros rész az aláhúzott szó után, attól még az utána lévõ szóköz nincs aláhúzva!
    if (red_len) {
        rect = word_rects[i];
        SDL_Rect space_rect = {x, y, rect.x + rect.w - x, rect.h};
        render_string_to_rect_shaded(" ", color1, color3, font, space_rect, renderer);
        red_len--;
    }
    if (i < text.word_count-1) {
        i++;
    } else {
        red_len = 0;
    }
    //addig írunk piros háttérrel szavakat, amíg az összes helytelen karaktert ki nem színeztük
    while (red_len) {
        rect = word_rects[i];
        word = text.words[i];
        int word_len = strlen(word);
        if (red_len > word_len) { //az egész szó helytelen (piros)
            strcpy(display_str, word);
            strcat(display_str, " ");
            render_string_to_rect_shaded(display_str, color1, color3, font, rect, renderer);
            red_len -= (word_len + 1);
        } else { //a szó egy része piros, egy része fekete
            strcpy(display_str, word);
            display_str[red_len] ='\0';
            x = rect.x;
            y = rect.y;
            rect = render_string_shaded(display_str, color1, color3, font, x, y, renderer);
            x += rect.w;
            render_string_blended(add_space(display_str, word+red_len), color1, font, x, y, renderer);
            x += rect.w;
            red_len = 0;
        }
        i++;
    }
    for (i=cursor_index+1; i<text.word_count; i++) {
        //a még nem begépelt szavakat feketével írjuk ki
        word = text.words[i];
        rect = word_rects[i];
        render_string_to_rect_blended(add_space(display_str, word), color1, font, rect, renderer);
    }

}

//kirajzol egy kocsi ábrát
void render_car(SDL_Renderer* renderer, SDL_Color color1, SDL_Color color2, int x, int y, int w, int h) {
    Sint16 vx[8] = {x, x, x+w/8, x+w*3/8, x+w*5/8, x+w*7/8, x+w, x+w};
    Sint16 vy[8]= {y+h*4/5, y+h*2/5, y+h*2/5, y, y, y+h*2/5, y+h*2/5, y+h*4/5};
    filledPolygonRGBA(renderer, vx, vy, 8, color1.r, color1.g, color1.b, 255); //test
    polygonRGBA(renderer, vx, vy, 8, color2.r, color2.g, color2.b, 255); //test körvonal
    filledCircleRGBA(renderer, x+w/4, y+h*4/5, h/5, color1.r, color1.g, color1.b, 255); //bal kerék
    filledCircleRGBA(renderer, x+w/4, y+h*4/5, h/10, color2.r, color2.g, color2.b, 255); //bal kerék tengely
    circleRGBA(renderer, x+w/4, y+h*4/5, h/5, color2.r, color2.g, color2.b, 255); //bal kerék körvonal
    filledCircleRGBA(renderer, x+w/4*3, y+h*4/5, h/5, color1.r, color1.g, color1.b, 255); //jobb kerék
    filledCircleRGBA(renderer, x+w/4*3, y+h*4/5, h/10, color2.r, color2.g, color2.b, 255); //jobb kerék tengely
    circleRGBA(renderer, x+w/4*3, y+h*4/5, h/5, color2.r, color2.g, color2.b, 255); //jobb kerék körvonal
    //hatso ablak:
    Sint16 vx_hablak[3] = {x+w*1.7/8, x+w*3.25/8, x+w*3.25/8};
    Sint16 vy_hablak[3] = {y+h*1.75/5, y+h*0.25/5, y+h*1.75/5};
    filledPolygonRGBA(renderer, vx_hablak, vy_hablak, 3, color2.r, color2.g, color2.g, 255);
    //kozepso ablak:
    Sint16 vx_kablak[4] = {x+w*3.4/8, x+w*4.6/8, x+w*4.6/8, x+w*3.4/8};
    Sint16 vy_kablak[4] = {y+h*0.25/5, y+h*0.25/5, y+h*1.75/5, y+h*1.75/5};
    filledPolygonRGBA(renderer, vx_kablak, vy_kablak, 4, color2.r, color2.g, color2.g, 255);
    //elso ablak:
    Sint16 vx_eablak[3] = {x+w*4.75/8, x+w*6.3/8, x+w*4.75/8,};
    Sint16 vy_eablak[3] = {y+h*0.25/5, y+h*1.75/5, y+h*1.75/5};
    filledPolygonRGBA(renderer, vx_eablak, vy_eablak, 3, color2.r, color2.g, color2.g, 255);

}
