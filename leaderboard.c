#include <string.h>

#include "engine.h"
#include "game.h"


int order_players(const void * a, const void * b) {
    return ((Player *)a)->score - ((Player *)b)->score;
}

void write_leaderboard(Player leaderboard[LEADERBOARD_SIZE]) {
    FILE * file = fopen(LEADERBOARD_FILENAME, "w");
    if (file == NULL)
        return;
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        if (strlen(leaderboard[i].name) == 0)
            break;
        fprintf(file, "%lu %s\n",
            leaderboard[i].score,
            leaderboard[i].name
        );
    }
    fclose(file);
}

int read_leaderboard(Player leaderboard[LEADERBOARD_SIZE]) {
    FILE * file = fopen(LEADERBOARD_FILENAME, "r");

    if (file == NULL)
        return 0;

    int leaderboard_count = 0;
    while (fscanf(file, "%lu %19[^\n]",
        &leaderboard[leaderboard_count].score,
        leaderboard [leaderboard_count].name
    ) == 2) leaderboard_count++;
    fclose(file);

    if (leaderboard_count > 1)
        qsort(leaderboard, leaderboard_count, sizeof(Player), order_players);
    return leaderboard_count;
}

void add_player_to_leaderboard(
    const char * name,
    unsigned long score
) {
    // TODO: to allow continuous play change this to game->leaderboard
    Player leaderboard[LEADERBOARD_SIZE] = { 0 };
    int i, leaderboard_count = read_leaderboard(leaderboard);

    // assume players are sorted from lowest to highest point
    for (i = 0; i < leaderboard_count; i++) {
        if (score < leaderboard[i].score)
            break; // insert player at (i)
    }

    if (i == LEADERBOARD_SIZE)
        return; // current player scored the highest

    // move remaining players back
    memmove(
        leaderboard + i + 1, leaderboard + i,
        sizeof (Player) * (LEADERBOARD_SIZE - i - 1)
    );
    // set leaderboard data
    strcpy(leaderboard[i].name, name);
    leaderboard[i].score = score;

    write_leaderboard(leaderboard);
}

void read_player_name(struct Game * game) {
    WINDOW * w = game->window;
    curs_set(1);
    echo();

    keypad(w,  FALSE);
    nodelay(w, FALSE);

    int posy, posx;
    getmaxyx(w, posy, posx);
    posy = (posy - TEXT_BOX_HEIGHT) / 2;
    posx = (posx - TEXT_BOX_WIDTH)  / 2;
    char name[MAX_NAME_SIZE] = { 0 };
    
    int center = empty_message_box(w, posy, posx)
               - TEXT_BOX_HEIGHT / 2 - 1;
    
    mvwaddstr (w, center-1, posx + 8, "+  Winner  +");
    mvwaddstr (w, center,   posx + 2, "Name:");
    mvwgetnstr(w, center,   posx + 7, name, MAX_NAME_SIZE - 1);

    if (strlen(name) > 0) {
        add_player_to_leaderboard(name, game->score);
    }
}