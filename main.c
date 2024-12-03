#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "engine.h"
#include "strip.h"
#include "game.h"

#define STORK_IMPLEMENTATION
#include "stork.h"

extern void read_player_name(struct Game * game);

/*
 * IIII NN   N IIII TTTTTT 
 *  II  NNN  N  II  TTTTTT 
 *  II  N NN N  II    TT   
 *  II  N  NNN  II    TT   
 * IIII N   NN IIII   TT 
 */

int define_new_color(short r, short g, short b) {
    static int color_count = 0;
    int index = COLOR_OFFSET - ++color_count;
    init_color(index, r, g, b);
    return -color_count;
}

void init_custom_colors(const struct Game * game) {    
    INFO("Colors: %d %d %d %d %d",
        has_colors(),
        COLORS, COLOR_PAIRS,
        COLOR_BLACK, COLOR_WHITE
    );
    if (!can_change_color()) {
        init_pair(Null ,  COLOR_WHITE , COLOR_BLACK);
        init_pair(Frog ,  10          , COLOR_BLACK);
        init_pair(Tree ,  COLOR_GREEN , COLOR_BLACK);
        init_pair(Water,  COLOR_CYAN  , COLOR_BLUE );
        init_pair(Log  ,  COLOR_YELLOW, COLOR_BLUE );
        init_pair(Car  ,  COLOR_RED   , COLOR_BLACK);
        init_pair(Taxi ,  COLOR_YELLOW, COLOR_BLACK);
        init_pair(Curb ,  COLOR_WHITE , COLOR_BLACK);
    } else {
        for (int i = 0; i < Symbol_Count; i++) {
            DEFINE_COLOR(i, game->colors[i][0], game->colors[i][1], game->colors[i][2]);
        }
    }
}

void init_sub_windows(struct Game * game, WINDOW * main_window) {
    int offx, offy;
    getmaxyx(stdscr, offy, offx);
    (void)offy;

    WINDOW * game_border = subwin(
        main_window,
        game->config.VISIBLE_STRIPS * CELL_HEIGHT + 2,
        game->size.x * CELL_WIDTH + 2,
        0, (offx - (game->size.x * CELL_WIDTH)) / 2 - 1
    );
    game->window = derwin(
        game_border,
        game->config.VISIBLE_STRIPS * CELL_HEIGHT,
        game->size.x * CELL_WIDTH,
        1, 1
    );
    
    game->info_panel = derwin(
        main_window,
        INFO_PANEL_HEIGHT, INFO_PANEL_WIDTH,
        game->config.VISIBLE_STRIPS * CELL_HEIGHT + 2,
        (offx - INFO_PANEL_WIDTH) / 2
    );

    nodelay(game->window, TRUE);
    keypad(game->window,  TRUE);

    box(game_border, '#', '#');
    box(game->info_panel, '#', '#');
    wrefresh(game_border);
    wrefresh(game->info_panel);
}
/*
 * MM   MM   AA   IIII NN   N LL     OOOOO   OOOOO  PPPPP  
 * MMM MMM  AAAA   II  NNN  N LL    OO   OO OO   OO PP  PP 
 * M MMM M  A  A   II  N NN N LL    OO   OO OO   OO PPPPP  
 * M  M  M AAAAAA  II  N  NNN LL    OO   OO OO   OO PP     
 * M     M AA  AA IIII N   NN LLLLL  OOOOO   OOOOO  PP     
 */

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
    } else
    if (keycode == ' ') {
        game->willing_to_travel ^= TRUE;
    }
}

void calculate_time_difference(
    struct timespec       * end,
    const struct timespec * start
) {
    // Calculate difference in seconds
    end->tv_sec -= start->tv_sec;
    
    // Calculate difference in nanoseconds
    if (end->tv_nsec < start->tv_nsec) {
        end->tv_sec -= 1; // Borrow 1 second
        end->tv_nsec = SECONDS * MICRO_SECONDS + end->tv_nsec - start->tv_nsec;
    } else {
        end->tv_nsec -= start->tv_nsec;
    }
}

void handle_frame(struct Game * game, int key) {
    render_game(game);
    render_game_state(game);
    
    wrefresh(game->info_panel);
    wrefresh(wgetparent(game->window));

    handle_key_down(game, key);
    update_game(game);
}

void main_loop(struct Game * game) {
    int key = ERR;
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    render_game(game);
    render_leaderboard(game);

    do {
        clock_gettime(CLOCK_REALTIME, &end);
        calculate_time_difference(&end, &start);

        int _key = wgetch(game->window);
        if (_key != ERR)
            key = _key;

        if (end.tv_sec > 0 || end.tv_nsec > game->config.TIMEOUT * MICRO_SECONDS) {
            handle_frame(game, key);

            clock_gettime(CLOCK_REALTIME, &start);
            key = ERR;
        }
        if (game->over == WIN) {
            game->level--;
            destroy_game(game);
            init_game(game);
            game->over = 0;
        }
    } while (game->level && game->over != LOSS);

    if (game->level == 0) game->over = WIN;
    render_game(game);
    render_game_state(game);
}

int main() {
    WINDOW * main_window = initscr();
    start_color();
    curs_set(0);
    noecho();

    struct Game game = { 0 };
    read_config_file(&game);
    const int seed = game.config.SEED ? game.config.SEED : time(NULL);
    srand(seed);
    init_game(&game);
    game.level = game.config.LEVEL_COUNT;
    init_custom_colors(&game);

    init_sub_windows(&game, main_window);

    main_loop(&game);

    INFO("Game Ended with %d", game.over);
    if (game.over == WIN) {
        read_player_name(&game);
    }

    destroy_game(&game);
    endwin();
    
    return 0;
}
