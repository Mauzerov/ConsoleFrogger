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
    echo();
    curs_set(1);

    keypad(w,  FALSE);
    nodelay(w, FALSE);

    int posy = (game->config.VISIBLE_STRIPS * CELL_HEIGHT - 5) / 2;
    int posx = (game->size.x * CELL_WIDTH - 28) / 2;
    char name[20] = { 0 };
    SET_COLOR_PAIR(Border, Border, Null);
    SET_TEXTCOLOR(w, Border);
    mvwaddstr (w, posy++, posx, "                            ");
    mvwaddstr (w, posy++, posx, " +======+  Winner  +======+ ");
    mvwaddstr (w, posy++, posx, " |Name:                   | ");
    mvwaddstr (w, posy++, posx, " +========================+ ");
    mvwaddstr (w, posy++, posx, "                            ");
    mvwgetnstr(w, posy-3, posx + 7, name, 19);

    if (strlen(name) > 0) {
        add_player_to_leaderboard(name, game->score);
    }
}