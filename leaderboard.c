#include <string.h>

#include "engine.h"
#include "game.h"


void render_leaderboard(struct Game * game) {
    WINDOW * window = game->info_panel;
    Player * leaderboard = game->leaderboard;

    mvwaddstr(window, 0, (INFO_PANEL_WIDTH - 12) >> 1, " SCOREBOARD ");
    
    char leaderboard_line[INFO_PANEL_WIDTH * 2] = { 0 };
    for (int i = 0; i < game->player_count; i++) {
        sprintf(
            leaderboard_line,
            " %1d: %-20s  : %05lu",
            i + 1, leaderboard[i].name, leaderboard[i].score
        );
        mvwaddstr(window, 1 + i, 2, leaderboard_line);
    }
}


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

    int player_count = 0;
    while (fscanf(file, "%lu %19[^\n]",
        &leaderboard[player_count].score,
        leaderboard [player_count].name
    ) == 2) player_count++;
    fclose(file);

    if (player_count > 1)
        qsort(leaderboard, player_count, sizeof(Player), order_players);
    return player_count;
}

void add_player_to_leaderboard(
    const char * name,
    unsigned long score
) {
    // TODO: to allow continuous play change this to game->leaderboard
    Player leaderboard[LEADERBOARD_SIZE] = { 0 };
    int i, player_count = read_leaderboard(leaderboard);

    // assume players are sorted from lowest to highest point
    for (i = 0; i < player_count; i++) {
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
    wattron(w, COLOR_PAIR(Null));
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