#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define HOSSZ 100
#define LEADERBOARD_SIZE 10
#define FILENAME "leaderboard.txt"

typedef struct LeaderboardEntry {
    double wpm;
    char name[HOSSZ];
} LeaderboardEntry;

typedef struct LeaderBoard {
    LeaderboardEntry entries[LEADERBOARD_SIZE];
    int num;
} LeaderBoard;

bool load_leaderboard(LeaderBoard* leaderboard) {
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL) {
        leaderboard->num = 0;
        return false;
    }
    fscanf(file, "%d", &(leaderboard->num));
    for (int i=0; i<leaderboard->num; i++) {
        fscanf(file, "%lf %s", &(leaderboard->entries[i].wpm), leaderboard->entries[i].name);
    }
    fclose(file);
    return true;
}

bool save_leaderboard(LeaderBoard leaderboard) {
    FILE* file = fopen(FILENAME, "w");
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

void clear_leaderboard(LeaderBoard* leaderboard) {
    leaderboard->num = 0;
}

void add_entry(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    int position = leaderboard->num;
    for (int i=0; i < leaderboard->num; i++) {
        if (entry.wpm > leaderboard->entries[i].wpm) {
            position = i;
            break;
        }
    }
    leaderboard->num = leaderboard->num < 10 ? leaderboard->num + 1 : 10;
    for (int i=leaderboard->num; i > position; i--) {
        leaderboard->entries[i].wpm = leaderboard->entries[i-1].wpm;
        strcpy(leaderboard->entries[i].name, leaderboard->entries[i-1].name);
    }
    leaderboard->entries[position].wpm = entry.wpm;
    strcpy(leaderboard->entries[position].name, entry.name);
}

int find_index(LeaderBoard leaderboard, LeaderboardEntry entry) {
    for (int i=0; i<leaderboard.num; i++) {
        if (strcmp(leaderboard.entries[i].name, entry.name) == 0) {
            return i;
        }
    }
    return -1;
}

void remove_entry(LeaderBoard* leaderboard, int idx) {
    if (leaderboard->num == 1) {
        leaderboard->num = 0;
    } else {
        leaderboard->num -= 1;
        for (int i=leaderboard->num-1; i>= idx; i--) {
            leaderboard->entries[i].wpm = leaderboard->entries[i+1].wpm;
            strcpy(leaderboard->entries[i].name, leaderboard->entries[i+1].name);
        }
    }
}

void update_leaderboard(LeaderBoard* leaderboard, LeaderboardEntry entry) {
    int idx = find_index(*leaderboard, entry);
    if (idx == -1) {
        add_entry(leaderboard, entry);
    }
    if (idx != -1 && leaderboard->entries[idx].wpm < entry.wpm) {
        remove_entry(leaderboard, idx);
        add_entry(leaderboard, entry);
    }

}

void print_leaderboard(LeaderBoard leaderboard) {
    printf("----------------\nThe leaderboard:\n");
    for (int i=0; i<leaderboard.num; i++) {
        printf("Name: %s, wpm: %.2f\n", leaderboard.entries[i].name, leaderboard.entries[i].wpm);
    }
    printf("----------------\n");
}
int main() {
    LeaderBoard leaderboard;
    load_leaderboard(&leaderboard);
    clear_leaderboard(&leaderboard);
    print_leaderboard(leaderboard);

    LeaderboardEntry A = {4, "A"};
    update_leaderboard(&leaderboard, A);
    print_leaderboard(leaderboard);

    LeaderboardEntry B = {6, "B"};
    update_leaderboard(&leaderboard, B);
    print_leaderboard(leaderboard);

    LeaderboardEntry C = {3, "C"};
    update_leaderboard(&leaderboard, C);
    print_leaderboard(leaderboard);

    LeaderboardEntry D = {5, "D"};
    update_leaderboard(&leaderboard, D);
    print_leaderboard(leaderboard);

    LeaderboardEntry E = {7, "A"};
    update_leaderboard(&leaderboard, E);
    print_leaderboard(leaderboard);

    LeaderboardEntry F = {0, "B"};
    update_leaderboard(&leaderboard, F);
    print_leaderboard(leaderboard);

    save_leaderboard(leaderboard);
    return 0;
}
