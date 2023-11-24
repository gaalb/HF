#include "leaderboard.h"

/*
Betölti fájlból az elmentett ranglistát, a paraméterként átadott változóba.
Leaderboard* leaderboard: a változó pointere, ahova töltjük a ranglistát
Visszatérés: azt jelzi, sikeres volt-e a betöltés
*/
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
        //fscanf(file, "%lf %s", &(leaderboard->entries[i].wpm), leaderboard->entries[i].name); //ha csak egy szavas lehetne a név, ez jó lenne
    }
    fclose(file);
    return true;
}

/*
Kimenti a paraméterként kapott ranglistát fájlba. A fájlban az első sor a
ranglista hossza, a többi egy-egy helyezés, például:
3
Kovács Gábor
Break Elek
TótFerenc
LeaderBoard leaderboard: a ranglista amit kimentünk
Visszatérés: azt jelzi, sikeres volt-e a mentés (fájlművelet)
*/
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

/*
Beilleszt egy 'entry'-t a ranglistába, a megfelelõ helyre. Ha az
entry-nek nincs helye a ranglistán, nem csinál semmit.
Leaderboard* leaderboard: a ranglista pointere, amit módosítunk
LeaderboardEntry entry: a beszúrandó elem
*/
void add_entry(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    //ha a legutolsónál lassabb, nem kerül ranglistára
    if (leaderboard->num == 10 && entry.wpm < leaderboard->entries[leaderboard->num-1].wpm) {
        return;
    }
    int position = leaderboard->num;
    //melyik helyre kell tenni a ranglistán belül?
    for (int i=0; i < leaderboard->num; i++) {
        if (entry.wpm > leaderboard->entries[i].wpm) {
            position = i;
            break;
        }
    }
    //a ranglista a kijelöltnél hosszabb nem lehet
    leaderboard->num = leaderboard->num < LEADERBOARD_SIZE ? leaderboard->num + 1 : LEADERBOARD_SIZE;
    /*csináljunk helyet az új ranglistásnak: aki mögötte lesz, vigyünk
    eggyel lentebb a ranglistán, így beszúrhatjuk a helyére*/
    for (int i=leaderboard->num-1; i > position; i--) {
        leaderboard->entries[i].wpm = leaderboard->entries[i-1].wpm;
        strcpy(leaderboard->entries[i].name, leaderboard->entries[i-1].name);
    }
    leaderboard->entries[position].wpm = entry.wpm;
    strcpy(leaderboard->entries[position].name, entry.name);
}

/*
Kiveszi a ranglista idx-edik elemét, és a mögötte lévőket
előrébb hozza eggyel.
LeaderBoard leaderboard: a ranglista, amit módosítunk
int idx: az index, amelyet el kell távolítani
*/
void remove_entry(LeaderBoard* leaderboard, int idx) {
    if (leaderboard->num == 0) {
        return; //üres ranglistából nyilván nem lehet elemet kivenni
    } else {
        leaderboard->num -= 1;
        //minden utána lévő elemet hozzunk eggyel előrébb
        for (int i=idx; i<leaderboard->num; i++) {
            leaderboard->entries[i].wpm = leaderboard->entries[i+1].wpm;
            strcpy(leaderboard->entries[i].name, leaderboard->entries[i+1].name);
        }
    }

}

/*
Megmondja, a ranglistán hanyadik helyen áll az illető,
ha rajta van a ranglistán, vagy -1, ha nincs rajta
LeaderBoard leaderboard: a szóban forgó ranglista
LeaderboardEntry entry: a szóban forgó illető
Visszatérés: az illető helye, kivéve ha nincs rajta, mert akkor -1
*/
int find_index(LeaderBoard leaderboard, LeaderboardEntry entry) {
    for (int i=0; i<leaderboard.num; i++) {
        if (strcmp(leaderboard.entries[i].name, entry.name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
stdout-ra kirakja a ranglistát, csak debugoláshoz használatos
LeaderBoard leaderboard: a ranglista, amit kiíratunk
*/
void print_leaderboard(LeaderBoard leaderboard) {
    printf("----------------\nThe leaderboard:\n");
    for (int i=0; i<leaderboard.num; i++) {
        printf("Name: %s, wpm: %.2f\n", leaderboard.entries[i].name, leaderboard.entries[i].wpm);
    }
    printf("----------------\n");
}

/*
Frissíti a cím szerinti paraméterrel kapott ranglistát, a második
paraméterként kapott elemmel
Leaderboard* leaderboard: a ranglista pointere, amit frissítünk
LeaderboardEntry entry: a beszúrandó elem
*/
void update_leaderboard(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    int idx = find_index(*leaderboard, entry); //benne van?
    if (idx == -1) { //ha nincs benne, rakjuk bele
        add_entry(leaderboard, entry);
    }
    //ha már fenn volt a ranglistán, csak akkor rakjuk fel, ha javult az ideje
    if (idx != -1 && leaderboard->entries[idx].wpm < entry.wpm) {
        remove_entry(leaderboard, idx);
        add_entry(leaderboard, entry);
    }
    //mentsük azonnal fájlba is a ranglistát, hogy ha crashelne a program, ne vesszen el
    save_leaderboard(*leaderboard);
}

/*
Megmondja, egy adott wpm-mel ranglistás lenne-e valaki
LeaderBoard leaderboard: a szóban forgó ranglista
double wpm: a wpm amit vizsgálunk, dobogót érdemel-e
*/
bool top10(LeaderBoard leaderboard, double wpm) {
    //mivel WPM szerint csökkenő sorrendben vannak a ranglistán, az utolsó a leglassabb
    double slowest = leaderboard.entries[leaderboard.num-1].wpm;
    if (leaderboard.num < LEADERBOARD_SIZE || slowest < wpm) {
        return true;
    } else {
        return false;
    }
}


