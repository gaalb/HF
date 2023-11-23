#include "custom_renders.h"

/*
az s1 stringbe belemásolja az s2 stringet úgy, hogy konkatenál egy space-t
feltételezi, hogy elég hosszú s1, hogy elférjen benne az extra karakter
char* s1: a string, amiben az eredmény lesz
char*s2: a kiinduló string
Visszatérés: a módosított string pointere
*/

char* add_space(char* s1, char* s2) {
    strcpy(s1, s2);
    return strcat(s1, " ");
}

/*
megadja, két string közül az elsõ hány karater egyezik meg
char* target: a szó, amihez hasonlítunk
char* input: a szó, AMIT hasonlítunk
Visszatérés: az első egyező karakterek száma
Például:
target="ABCDEFGH"
input="ABC"
output: 3
*/
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

/*
megadja, input megfelel-e target első karaktereine
char* target: a szó, amihez hasonlítunk
char* input: a szó, AMIT hasonlítunk
Visszatérés: "helyes"-e a gépelés
Például:
target="ABCDEFGH"
input="ABC"
output: true
*/
bool input_correct(char* target, char* input) {
    int input_len = strlen(input);
    return match_len(target, input) == input_len;
}

/*
kirajzolja a szövegdobozt, ahova a bemenetet gépeljük, a benne lévõ szöveggel együtt
char* input: a string, ami éppen a bemenet
SDL_Rect teglalap: a szövegdoboz, ahova gépelünk
SDL_Color input_color: a szövegdoboz háttérszíne
TTF_Font* font: a használt betűtípus
SDL_Renderer* renderer: a használt renderer
char* composition: szövegbevitelnél használt segéd változó, az aktuális szerkesztést tartalmazza
char* textandcomposition: szövegbevitelnél használt segéd változó, ez az amit ki fogunk rajzolni
*/
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

/*
kirenderel egy stringet háttérszín nélkül egy megadott dobozba
char* str: a string amit kirajzol
SDL_Color color: a szín, amivel kirajzolja
TTF_Font* font: a betűtípus
SDL_Rect rect: a négyszög, amibe a szöveg kerül
SDL_Renderer* renderer: a használt renderer
*/
void render_string_to_rect_blended(char* str, SDL_Color color, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer) {
    SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, str, color);
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
}

/*
kirenderel egy stringet háttérszínnel egy megadott dobozba
char* str: a string amit kirajzol
SDL_Color color: a szín, amivel kirajzolja
SDL_Color background: a háttérszín
TTF_Font* font: a betűtípus
SDL_Rect rect: a négyszög, amibe a szöveg kerül
SDL_Renderer* renderer: a használt renderer
*/
void render_string_to_rect_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, SDL_Rect rect, SDL_Renderer* renderer) {
    SDL_Surface* word_s  = TTF_RenderUTF8_Shaded(font, str, color, background);
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
}

/*
kirenderel egy sztringet háttérszín nélkül egy adott x, y pozícióra, és
visszaadja a dobozt, amibe a sztring került
char* str: a string amit kirajzol
SDL_Color color: a szín, amivel kirajzolja
TTF_Font* font: a betűtípus
int x, y: az x és y pozíciója a szöveg kijelölt pontjának, a képernyő bal felső sarkához képest
SDL_Renderer* renderer: a használt renderer
Position position: kijelöli, a szöveg mely pontját adja x, y (pl. bal felső, közepe)
Visszatérés: a doboz, ami a keletkező felület helyét és kiterjedését megadja
*/
SDL_Rect render_string_blended(char* str, SDL_Color color, TTF_Font* font, int x, int y, SDL_Renderer* renderer, Position position) {
    SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, str, color);
    SDL_Rect rect;
    rect.w = word_s->w;
    rect.h = word_s->h;
    switch (position) {
        case TopLeft:
            rect.x = x;
            rect.y = y;
            break;
        case Middle:
            rect.x = x - rect.w/2;
            rect.y = y - rect.h/2;
            break;
        case Right:
            rect.y = y - rect.h/2;
            rect.x = x - rect.w;
            break;

    }
    x += rect.w;
    SDL_Texture* word_t = SDL_CreateTextureFromSurface(renderer, word_s);
    SDL_RenderCopy(renderer, word_t, NULL, &rect);
    SDL_FreeSurface(word_s);
    SDL_DestroyTexture(word_t);
    return rect;
}

/*
kirenderel egy sztringet háttérszínnel egy adott x, y pozícióra, és
visszaadja a dobozt, amibe a sztring került
char* str: a string amit kirajzol
SDL_Color color: a szín, amivel kirajzolja
SDL_Color background: a háttérszín
TTF_Font* font: a betűtípus
int x, y: az x és y pozíciója a szöveg kijelölt pontjának, a képernyő bal felső sarkához képest
SDL_Renderer* renderer: a használt renderer
Position position: kijelöli, a szöveg mely pontját adja x, y (pl. bal felső, közepe)
Visszatérés: a doboz, ami a keletkező felület helyét és kiterjedését megadja
*/
SDL_Rect render_string_shaded(char* str, SDL_Color color, SDL_Color background, TTF_Font* font, int x, int y, SDL_Renderer* renderer, Position position) {
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

/*
kiszámolja, a Text objektum melyik szavába esik éppen a kurzor, és cursor_len-be beleírja,
azon a szón belül hanyadik karakternél jár a kurzor
Text text: a szöveget tartalmazó struktúra
target_index: a sorszáma annak a szónak a szövegben, amit be kell gépelnie a játékosnak
int input_len: annak a szónak a hossza, ami be van gépelve a beviteli dobozba
int* cursor_len: a mutatott változóba beleírja a függvény, a szavon belül hanyadik
karakternél jár a kurzor (megjelenítéshez)
Visszatérés: az az index, amelyik szóban jár a kurzor text-ben
*/
int get_cursor_index(Text text, int target_index, int input_len, int* cursor_len) {
    int target_len = strlen(text.words[target_index]);
    while (input_len > target_len+1 && target_index < text.word_count-1) {
        target_index++;
        target_len += strlen(text.words[target_index])+1;
    }
    *cursor_len = input_len - target_len + strlen(text.words[target_index]);
    return target_index;
}

/*
kiszámolja, a szöveg szavai mely dobozokba kell, hogy essenek, ahhoz, hogy a szöveg beférjen a
megadott helyre, és helyet foglal a téglalpok tömbjének
int x: a szövegdoboz bal széle
int y: a szövegdoboz teteje
int w: a szövegdoboz szélessége
Text text: a szöveget tartalmazó struktúra
TTF_Font* font: a betűtípus
Visszatérés: SDL_Rect változók dinamikusan foglalt tömbjére mutató pointer
a szövegdoboz magassága abból dõl el, hány sorba fog kiférni a szöveg
Megj.: a foglalt területet fel kell szabadítani*/
SDL_Rect* calculate_Rects(Text text, TTF_Font* font, int x, int y, int w) {
    SDL_Color color = {0, 0, 0};
    int word_count = text.word_count;
    SDL_Rect* word_rects = (SDL_Rect*) malloc(word_count*sizeof(SDL_Rect));
    int right_edge = x+w; //ennél jobbrább nem végződhet egy szó sem
    int left_edge = x; //ide ugrunk vissza ha túl jobbra mennénk
    for (int i=0; i<word_count; i++) {
        char word[HOSSZ];
        SDL_Surface* word_s = TTF_RenderUTF8_Blended(font, add_space(word, text.words[i]), color);
        word_rects[i].w = word_s->w;
        word_rects[i].h = word_s->h;
        //ha túlságosan jobbra lenne a szöveg vége, akkor lentebb ugrunk, és visszamegyünk bal szélre
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

/*
Letörli a táblát, megadott színnel
GameData* game_data: a játék adatait tartalmazó struktúra pointere
SDL_Color color: a háttérszíne a játéknak
*/
void clear_screen(GameData* game_data, SDL_Color color) {
    boxRGBA(game_data->renderer, 0, 0, game_data->szeles, game_data->magas, color.r, color.g, color.b, 255);
}

/*
A kurzor index és a kurzor szavon belüli pozíciója alapján rendereli a kurzort (egy elnyújtott téglalap)
Text text: a szöveget tartalmazó struktúra
SDL_Rect* word_rects: a szavak elrendezését és méretét tartalmazó tömb
int cursor_index: a sorszáma a szónak, amiben a kurzor áll
int cursor_len: a kurzor helye a szavon belül
SDL_Renderer* renderer: a használt renderer
*/
void render_cursor(Text text, SDL_Rect* word_rects, TTF_Font* font, int cursor_index, int cusor_len, SDL_Renderer* renderer) {
    SDL_Color fekete = {0, 0, 0};
    SDL_Rect rect = word_rects[cursor_index];
    char display_str[HOSSZ];
    char *word = text.words[cursor_index];
    int x1, x2, y1, y2;
    if (cusor_len) {
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

/*
kirajzolja a szöveget
Text text: a szöveget tartalmazó struktúra
TTF_Font* font: a betűtípus
TTF_Font* underlined: ugyanaz mint font, csak aláhúzva
SDL_Rect* word_rects: a szavak elrendezését és méretét tartalmazó tömb
SDL_Renderer* renderer: a használt renderer
SDL_Color color1: azoknak a szavaknak a színe, amik a kurzor után vannak (fekete)
SDL_Color color2: azoknak a szavaknak a színe, amik már helyesen be lettek írva (zöld)
SDL_Color color3: a háttérszín amivel jelöljük a helytelen gépelést (piros)
int target_index: annak a szónak az indexe, amit éppen be kell gépelni
char* input: a játékos által begépelt szó
*/
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
        rect = render_string_blended(display_str, color2, underlined, x, y, renderer, TopLeft);
        x += rect.w;
    } //pirossal aláhúzott rész
    if (red_target_len) {
        strcpy(display_str, target+green_target_len);
        display_str[red_target_len] = '\0';
        rect = render_string_shaded(display_str, color1, color3, underlined, x, y, renderer, TopLeft);
        x += rect.w;
    } //feketével aláhúzott rész
    if (black_target_len) {
        strcpy(display_str, target+green_target_len+red_target_len);
        rect = render_string_blended(display_str, color1, underlined, x, y, renderer, TopLeft);
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
            rect = render_string_shaded(display_str, color1, color3, font, x, y, renderer, TopLeft);
            x += rect.w;
            render_string_blended(add_space(display_str, word+red_len), color1, font, x, y, renderer, TopLeft);
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

/*
kirajzol egy kocsit
Car car: a rajzolandó kocsi adatait tároló struktúra
SDL_Renderer* renderer: a használt renderer
TTF_Font* font: a betűtípus
*/
void render_car(SDL_Renderer* renderer, Car car, TTF_Font* font) {
    Sint16 vx[8] = {car.x, car.x, car.x+car.w/8, car.x+car.w*3/8, car.x+car.w*5/8, car.x+car.w*7/8, car.x+car.w, car.x+car.w};
    Sint16 vy[8]= {car.y+car.h*4/5, car.y+car.h*2/5, car.y+car.h*2/5, car.y, car.y, car.y+car.h*2/5, car.y+car.h*2/5, car.y+car.h*4/5};
    filledPolygonRGBA(renderer, vx, vy, 8, car.color1.r, car.color1.g, car.color1.b, 255); //test
    polygonRGBA(renderer, vx, vy, 8, car.color2.r, car.color2.g, car.color2.b, 255); //test körvonal
    filledCircleRGBA(renderer, car.x+car.w/4, car.y+car.h*4/5, car.h/5, car.color1.r, car.color1.g, car.color1.b, 255); //bal kerék
    filledCircleRGBA(renderer, car.x+car.w/4, car.y+car.h*4/5, car.h/10, car.color2.r, car.color2.g, car.color2.b, 255); //bal kerék tengely
    circleRGBA(renderer, car.x+car.w/4, car.y+car.h*4/5, car.h/5, car.color2.r, car.color2.g, car.color2.b, 255); //bal kerék körvonal
    filledCircleRGBA(renderer, car.x+car.w/4*3, car.y+car.h*4/5, car.h/5, car.color1.r, car.color1.g, car.color1.b, 255); //jobb kerék
    filledCircleRGBA(renderer, car.x+car.w/4*3, car.y+car.h*4/5, car.h/10, car.color2.r, car.color2.g, car.color2.b, 255); //jobb kerék tengely
    circleRGBA(renderer, car.x+car.w/4*3, car.y+car.h*4/5, car.h/5, car.color2.r, car.color2.g, car.color2.b, 255); //jobb kerék körvonal
    //hatso ablak:
    Sint16 vx_hablak[3] = {car.x+car.w*1.7/8, car.x+car.w*3.25/8, car.x+car.w*3.25/8};
    Sint16 vy_hablak[3] = {car.y+car.h*1.75/5, car.y+car.h*0.25/5, car.y+car.h*1.75/5};
    filledPolygonRGBA(renderer, vx_hablak, vy_hablak, 3, car.color2.r, car.color2.g, car.color2.g, 255);
    //kozepso ablak:
    Sint16 vx_kablak[4] = {car.x+car.w*3.4/8, car.x+car.w*4.6/8, car.x+car.w*4.6/8, car.x+car.w*3.4/8};
    Sint16 vy_kablak[4] = {car.y+car.h*0.25/5, car.y+car.h*0.25/5, car.y+car.h*1.75/5, car.y+car.h*1.75/5};
    filledPolygonRGBA(renderer, vx_kablak, vy_kablak, 4, car.color2.r, car.color2.g, car.color2.g, 255);
    //elso ablak:
    Sint16 vx_eablak[3] = {car.x+car.w*4.75/8, car.x+car.w*6.3/8, car.x+car.w*4.75/8,};
    Sint16 vy_eablak[3] = {car.y+car.h*0.25/5, car.y+car.h*1.75/5, car.y+car.h*1.75/5};
    filledPolygonRGBA(renderer, vx_eablak, vy_eablak, 3, car.color2.r, car.color2.g, car.color2.g, 255);
    SDL_Color fekete = {0, 0, 0};
    render_string_blended(car.name, fekete, font, car.x, car.y+car.h/2, renderer, Right);
}

/*
kirajzolja a ranglistát
GameData* game_data: a játék adatait tartalmazó struktúra pointere
SDL_Color color1: a betűszín
SDL_Color color2: a háttérszín
*/
void render_leaderboard(GameData* game_data, SDL_Color color1, SDL_Color color2) {
    int top = (int)(game_data->margo);
    int bottom = (int)(game_data->magas - game_data->margo);
    int left = (int)(game_data->szeles*0.4 + game_data->margo);
    int right = (int)(game_data->szeles - game_data->margo);
    LeaderBoard leaderboard = game_data->leaderboard;
    TTF_Font* font = game_data->font;
    SDL_Renderer* renderer = game_data->renderer;
    boxRGBA(renderer, left, top, right, bottom, color2.r, color2.g, color2.b, 255);
    rectangleRGBA(renderer, left, top, right, bottom, color1.r, color1.g, color1.b, 255);
    SDL_Rect rect = render_string_blended("Leaderboard:", color1, font, left, top, renderer, TopLeft);
    hlineRGBA(renderer, left, right, game_data->margo+60, color1.r, color1.g, color1.b, 255);
    top += rect.h + game_data->margo;
    int h = (bottom - top) / LEADERBOARD_SIZE;
    for (int i=0; i<leaderboard.num; i++) {
        char display_str[2*HOSSZ];
        sprintf(display_str, "%d.: %s, WPM: %.2f", i+1, leaderboard.entries[i].name, leaderboard.entries[i].wpm);
        render_string_blended(display_str, color1, font, (left+right)/2, top+h*(i+0.5), renderer, Middle);
    }
}

/*kirajzol egy gombot
Button button: a kirajzolandó gomb struktúrája
SDL_Renderer* renderer: a használt renderer
TTF_Font* font: a gomb szövegének betűtípusa
Megj.: vigyázat, ha a szöveg túl nagy, kilóghat a dobozából a gombnak!
*/
void render_button(Button button, SDL_Renderer* renderer, TTF_Font *font) {
    boxRGBA(renderer, button.rect.x, button.rect.y, button.rect.x + button.rect.w, button.rect.y + button.rect.h, button.color.r, button.color.g, button.color.b, 255);
    rectangleRGBA(renderer, button.rect.x, button.rect.y, button.rect.x + button.rect.w, button.rect.y + button.rect.h, 0, 0, 0, 255);
    render_string_blended(button.str, button.str_color, font, button.rect.x + button.rect.w/2, button.rect.y + button.rect.h/2, renderer, Middle);
}

/*
Kirajzolja a piros-sárga-zöld lámpát, lépteti a címként átvett countdown változót, és
átállítja igaz-ra a címként átvett countdown_over változót, ha lejárt a visszaszámlálás
bool* countdown_over: a változó pointere, amiben jelöljük a visszaszámlálás lejárásának tényét
SDL_Rect rect: a lámpa doboz téglalapja
TTF_Font* font: a visszaszámlálás betűtípusa
clock_t* t: a visszaszámlálást pörgető változó (ebben van az idő tárolva)
SDL_Renderer* renderer: a használt renderer
*/
void handle_countdown_s(bool* countdown_over, SDL_Rect rect, int* countdown, TTF_Font *font, clock_t* t, SDL_Renderer* renderer) {
    if (countdown >= 0) {
        SDL_Color sarga = {255, 255, 0};
        SDL_Color zold = {0, 255, 0};
        SDL_Color piros = {255, 50, 50};
        SDL_Color szurke = {195, 195, 195};
        SDL_Color sotet_szurke = {100, 100, 100};
        SDL_Color fekete = {0, 0, 0};
        SDL_Color color;
        boxRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, szurke.r, szurke.g, szurke.b, 255);
        rectangleRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, fekete.r, fekete.g, fekete.b, 255);
        char num[HOSSZ] = "";
        sprintf(num, "%.2d", *countdown);
        render_string_blended(num, fekete, font, rect.x + rect.w*7/8, rect.y + rect.h/2, renderer, Middle);
        if (*countdown > 3) {
            color = piros;
        } else {
            color = sotet_szurke;
        }
        filledCircleRGBA(renderer, rect.x + rect.w*5/8, rect.y + rect.h/2, rect.w/8.5, color.r, color.g, color.b, 255);
        circleRGBA(renderer, rect.x + rect.w*5/8, rect.y + rect.h/2, rect.w/8.5, fekete.r, fekete.g, fekete.b, 255);
        if (*countdown <= 3 && *countdown >0) {
            color = sarga;
        } else {
            color = sotet_szurke;
        }
        filledCircleRGBA(renderer, rect.x + rect.w*3/8, rect.y + rect.h/2, rect.w/8.5, color.r, color.g, color.b, 255);
        circleRGBA(renderer, rect.x + rect.w*3/8, rect.y + rect.h/2, rect.w/8.5, fekete.r, fekete.g, fekete.b, 255);
        if (*countdown == 0) {
            color = zold;
            *countdown_over = true;
            *t = clock();
        } else {
            color = sotet_szurke;
        }
        filledCircleRGBA(renderer, rect.x + rect.w/8, rect.y + rect.h/2, rect.w/8.5, color.r, color.g, color.b, 255);
        circleRGBA(renderer, rect.x + rect.w/8, rect.y + rect.h/2, rect.w/8.5, fekete.r, fekete.g, fekete.b, 255);
        *countdown -= 1;
    }
}

/*
Kirajzolja a multiplayer játékosok neveit
GameData* game_data: a játék adatait tartalmazó struktúra pointere
SDL_Color color1: a betűszín
SDL_Color color2: a háttérszín
*/
void render_players(GameData* game_data, SDL_Color color1, SDL_Color color2) {
    int top = (int)(game_data->margo);
    int bottom = (int)(game_data->magas - game_data->margo);
    int left = (int)(game_data->szeles*0.5 + game_data->margo);
    int right = (int)(game_data->szeles - game_data->margo);
    TTF_Font* font = game_data->font;
    SDL_Renderer* renderer = game_data->renderer;
    boxRGBA(renderer, left, top, right, bottom, color2.r, color2.g, color2.b, 255);
    rectangleRGBA(renderer, left, top, right, bottom, color1.r, color1.g, color1.b, 255);
    SDL_Rect rect = render_string_blended("Players:", color1, font, left, top, renderer, TopLeft);
    top += rect.h + game_data->margo;
    int h = (bottom - top) / 8;
    for (int i=0; i<5; i++) {
        rectangleRGBA(renderer, left, top+h*i, right, top+h*(i+1), color1.r, color1.g, color1.b, 255);
    }
    for (int i=0; i<game_data->players; i++) {
        render_string_blended(game_data->multis[i].car.name, color1, font, (left+right)/2, top+h*(i+0.5), renderer, Middle);
    }
}

/*
Kirajzolja a botok beállításait
GameData* game_data: a játék adatait tartalmazó struktúra pointere
SDL_Color color1: a betűszín
SDL_Color color2: a háttérszín
*/
void render_settings(GameData* game_data, SDL_Color color1, SDL_Color color2) {
    int top = (int)(game_data->margo);
    int bottom = (int)(game_data->magas - game_data->margo);
    int left = (int)(game_data->margo);
    int right = (int)(game_data->szeles*0.4);
    TTF_Font* font = game_data->font;
    SDL_Renderer* renderer = game_data->renderer;
    boxRGBA(renderer, left, top, right, bottom, color2.r, color2.g, color2.b, 255);
    rectangleRGBA(renderer, left, top, right, bottom, color1.r, color1.g, color1.b, 255);
    SDL_Rect rect = render_string_blended("Settings:", color1, font, left, top, renderer, TopLeft);
    hlineRGBA(renderer, left, right, game_data->margo+60, color1.r, color1.g, color1.b, 255);
    top += rect.h + game_data->margo;
    int h = (bottom-top) / LEADERBOARD_SIZE; //esztétkailag ez a szép, habár nincs köze a ranglistához
    for (int i=0; i<4; i++) {
        char display_str[2*HOSSZ];
        sprintf(display_str, "%s WPM: %.0f", game_data->bots[i].car.name, game_data->bots[i].expected_wpm);
        render_string_blended(display_str, color1, font, (left+right)/2, top+h*(i+0.5), renderer, Middle);
    }
}
