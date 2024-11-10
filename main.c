#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "engine.h"
#include "strip.h"
#include "game.h"


void handle_key_down(struct Game * game, int keycode) {
    memset(&game->prev_move, 0, sizeof(struct Point));
    if (keycode == 'w') {
        game->player.y +=
            game->prev_move.y = -(game->player.y > 0);
    } else
    if (keycode == 's') {
        game->player.y +=
            game->prev_move.y = +(game->player.y + 1 < GAME_HEIGHT);
    } else
    if (keycode == 'd') {
        game->player.x +=
            game->prev_move.x = +(game->player.x + 1 < GAME_WIDTH);
    } else
    if (keycode == 'a') {
        game->player.x +=
            game->prev_move.x = -(game->player.x > 0);
    }
}

int main() {
    srand(time(NULL));
    srand(rand());
    srand(rand());
    struct Game game = { 0 };

    init_game(&game);

    initscr();
    timeout(800); // TODO: Add Const/Define 
    start_color();

    init_pair(Null ,  COLOR_WHITE , COLOR_BLACK);
    init_pair(Frog ,  10          , COLOR_BLACK);
    init_pair(Tree ,  COLOR_GREEN , COLOR_BLACK);
    init_pair(Water,  COLOR_CYAN  , COLOR_BLUE );
    init_pair(Log  ,  COLOR_YELLOW, COLOR_BLUE );
    init_pair(Car  ,  COLOR_RED   , COLOR_BLACK);
    init_pair(Taxi ,  COLOR_GREEN , COLOR_BLACK);
    init_pair(Curb ,  COLOR_WHITE , COLOR_BLACK);

    curs_set(0);
    noecho();
    int key = ERR;

    int time_passed = 0;
    do {
        fprintf(stderr, "Rendering %d\n", time_passed);
        render_game(&game);
        key = getch();
        refresh();
        handle_key_down(&game, key);

        update_game(&game);    
    } while (!game.over);

    destroy_game(&game);
    endwin();
    
    return 0;
}