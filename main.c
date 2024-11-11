#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "engine.h"
#include "strip.h"
#include "game.h"

int order_players(const void * a, const void * b) {
    return ((Player *)a)->score - ((Player *)b)->score;
}

void write_leaderboard(FILE * file, Player leaderboard[LEADERBOARD_SIZE]) {
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        if (strlen(leaderboard[i].name) == 0)
            break;
        fprintf(file, "%s %lu\n", leaderboard[i].name, leaderboard[i].score);
    }
}

int read_leaderboard(FILE * file, Player leaderboard[LEADERBOARD_SIZE]) {
    int player_count = 0;
    while (fscanf(file, "%19s %lu",
        leaderboard [player_count].name,
        &leaderboard[player_count].score
    ) == 2) player_count++;

    if (player_count > 1)
        qsort(leaderboard, player_count, sizeof(Player), order_players);
    return player_count;
}

void add_player_to_leaderboard(
    FILE * file,
    const char * name,
    unsigned long score
) {
    Player leaderboard[LEADERBOARD_SIZE] = { 0 };
    int i, player_count = read_leaderboard(file, leaderboard);

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

    write_leaderboard(
        freopen(NULL, "w", file),
        leaderboard
    );
}

void read_player_name(struct Game * game) {
    echo();
    curs_set(1);
    timeout(-1);
    struct Point off = get_offset(game);

    int posy = off.y + (game->size.y / 2) - 2;
    int posx = off.x + (game->size.x / 2) - 12;
    char name[20] = { 0 };

    mvaddstr(posy++, posx, "                            ");
    mvaddstr(posy++, posx, " +======+  Winner  +======+ ");
    mvaddstr(posy++, posx, " |Name:                   | ");
    mvaddstr(posy++, posx, " +========================+ ");
    mvaddstr(posy++, posx, "                            ");
    mvscanw (posy-3, posx + 7, "%19s", name);

    FILE * file = fopen(LEADERBOARD_FILENAME, "a+");
    if (file != NULL && strlen(name) > 0) {
        add_player_to_leaderboard(file, name, game->score);
        fclose(file);
    }
}

void handle_key_down(struct Game * game, int keycode) {
    memset(&game->prev_move, 0, sizeof(struct Point));
    if (keycode == 'w') {
        game->player.y +=
            game->prev_move.y = -(game->player.y > 0);
    } else
    if (keycode == 's') {
        game->player.y +=
            game->prev_move.y = +(game->player.y + 1 < game->size.y);
    } else
    if (keycode == 'd') {
        game->player.x +=
            game->prev_move.x = +(game->player.x + 1 < game->size.x);
    } else
    if (keycode == 'a') {
        game->player.x +=
            game->prev_move.x = -(game->player.x > 0);
    }
}

void init_curses(const struct Config * config) {
    initscr();
    timeout(config->TIMEOUT);
    start_color();

    init_pair(Null ,  COLOR_WHITE , COLOR_BLACK);
    init_pair(Frog ,  10          , COLOR_BLACK);
    init_pair(Tree ,  COLOR_GREEN , COLOR_BLACK);
    init_pair(Water,  COLOR_CYAN  , COLOR_BLUE );
    init_pair(Log  ,  COLOR_YELLOW, COLOR_BLUE );
    init_pair(Car  ,  COLOR_RED   , COLOR_BLACK);
    init_pair(Taxi ,  COLOR_YELLOW, COLOR_BLACK);
    init_pair(Curb ,  COLOR_WHITE , COLOR_BLACK);

    curs_set(0);
    noecho();
}

int main() {
    srand(time(NULL));
    srand(rand());
    srand(rand());
    struct Game game = { 0 };

    init_game(&game);
    init_curses(&game.config);
    int key = ERR;

    do {
        render_game(&game);
        key = getch();
        refresh();
        handle_key_down(&game, key);

        update_game(&game);    
    } while (!game.over);

    render_game(&game);
    if (game.over == WIN) {
        read_player_name(&game);
    }

    destroy_game(&game);
    endwin();
    
    return 0;
}