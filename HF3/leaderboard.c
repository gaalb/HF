#include "leaderboard.h"

/*Bet�lti f�jlb�l az elmentett ranglist�t, a param�terk�nt �tadott
v�ltoz�ba. A visszat�r�s jelzi a m�velet sikeress�g�t*/
bool load_leaderboard(LeaderBoard* leaderboard) {
    FILE* file = fopen(LEADERBOARD, "r");
    if (file == NULL) {
        leaderboard->num = 0;
        return false;
    }
    fscanf(file, "%d", &(leaderboard->num));
    for (int i=0; i<leaderboard->num; i++) {
        fscanf(file, "%lf ", &(leaderboard->entries[i].wpm));
        fgets(leaderboard->entries[i].name, HOSSZ, file);
        leaderboard->entries[i].name[strlen(leaderboard->entries[i].name)-1] = '\0'; //kiszedi a '\n'-t
        //fscanf(file, "%lf %s", &(leaderboard->entries[i].wpm), leaderboard->entries[i].name);
    }
    fclose(file);
    return true;
}

/*Kimenti a param�terk�nt kapott ranglist�t f�jlba.
A f�jlban az els� sor a ranglista hossza, a t�bbi egy-egy
helyez�s: WPM N�V*/
bool save_leaderboard(LeaderBoard leaderboard) {
    FILE* file = fopen(LEADERBOARD, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file for writing\n");
        return false;
    }
    fprintf(file, "%d\n", leaderboard.num);
    for (int i=0; i<leaderboard.num; i++) {
        fprintf(file, "%.2lf %s\n", leaderboard.entries[i].wpm, leaderboard.entries[i].name);
    }
    fclose(file);
    return true;
}

/*Beilleszt egy 'entry'-t a ranglist�ba, a megfelel� helyre. Ha az
entry-nek nincs helye a ranglist�n, nem csin�l semmit, de egy�bk�nt
norm�l esetben nem is h�vjuk meg ha nem kell ranglist�ra tenni.*/
void add_entry(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    //ha a legutols�n�l lassabb, nem ker�l ranglist�ra
    if (leaderboard->num == 10 && entry.wpm < leaderboard->entries[leaderboard->num-1].wpm) {
        return;
    }
    int position = leaderboard->num;
    //melyik helyre kell tenni a ranglist�n bel�l?
    for (int i=0; i < leaderboard->num; i++) {
        if (entry.wpm > leaderboard->entries[i].wpm) {
            position = i;
            break;
        }
    }
    //a ranglista a kijel�ltn�l hosszabb nem lehet
    leaderboard->num = leaderboard->num < LEADERBOARD_SIZE ? leaderboard->num + 1 : LEADERBOARD_SIZE;
    /*csin�ljunk helyet az �j ranglist�snak: aki m�g�tte lesz, vigy�nk
    eggyel lentebb a ranglist�n, �gy bet�zhetj�k a hely�re*/
    for (int i=leaderboard->num-1; i > position; i--) {
        leaderboard->entries[i].wpm = leaderboard->entries[i-1].wpm;
        strcpy(leaderboard->entries[i].name, leaderboard->entries[i-1].name);
    }
    leaderboard->entries[position].wpm = entry.wpm;
    strcpy(leaderboard->entries[position].name, entry.name);
}

/*Kiveszi a ranglista idx-edik elem�t, �s a m�g�tte l�v�ket
el�r�bb hozza eggyel.*/
void remove_entry(LeaderBoard* leaderboard, int idx) {
    if (leaderboard->num == 0) {
        return;
    }
    if (leaderboard->num == 1) {
        leaderboard->num = 0;
    } else {
        leaderboard->num -= 1;
        for (int i=idx; i<leaderboard->num; i++) {
            leaderboard->entries[i].wpm = leaderboard->entries[i+1].wpm;
            strcpy(leaderboard->entries[i].name, leaderboard->entries[i+1].name);
        }
    }
}

/*Megmondja, a ranglist�n hanyadik helyen �ll az illet�,
ha rajta van a ranglist�n, vagy -1, ha nincs rajta*/
int find_index(LeaderBoard leaderboard, LeaderboardEntry entry) {
    for (int i=0; i<leaderboard.num; i++) {
        if (strcmp(leaderboard.entries[i].name, entry.name) == 0) {
            return i;
        }
    }
    return -1;
}

/*stdout-ra kirakja a ranglist�t, csak debugol�shoz haszn�latos*/
void print_leaderboard(LeaderBoard leaderboard) {
    printf("----------------\nThe leaderboard:\n");
    for (int i=0; i<leaderboard.num; i++) {
        printf("Name: %s, wpm: %.2f\n", leaderboard.entries[i].name, leaderboard.entries[i].wpm);
    }
    printf("----------------\n");
}

/*Friss�ti a c�m szerinti param�terrel kapott ranglist�t, a m�sodik
param�terk�nt kapott elemmel*/
void update_leaderboard(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    int idx = find_index(*leaderboard, entry); //benne van?
    if (idx == -1) { //ha nincs benne, rakjuk bele
        add_entry(leaderboard, entry);
    }
    //ha m�r fenn volt a ranglist�n, csak akkor rakjuk fel, ha javult az ideje
    if (idx != -1 && leaderboard->entries[idx].wpm < entry.wpm) {
        remove_entry(leaderboard, idx);
        add_entry(leaderboard, entry);
    }
    //print_leaderboard(*leaderboard);

}

/*Megmondja, egy adott wpm-mel ranglist�s lenne-e valaki*/
bool top10(LeaderBoard leaderboard, double wpm) {
    double slowest = leaderboard.entries[leaderboard.num-1].wpm;
    if (leaderboard.num < LEADERBOARD_SIZE || slowest < wpm) {
        return true;
    } else {
        return false;
    }
}

