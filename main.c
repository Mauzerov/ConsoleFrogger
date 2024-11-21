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

WINDOW * init_curses(const struct Config * config) {
    (void)config;
    WINDOW * window = initscr();
    // wtimeout(window, 0);
    fprintf(stderr, "Colors: %d\n", has_colors());
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
    return window;
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

    // cbreak();
    nodelay(game->window, TRUE);
    keypad(game->window,  TRUE);

    box(game_border, '#', '#');
    box(game->info_panel, '#', '#');
    wrefresh(game_border);
    wrefresh(game->info_panel);
}


void calculate_time_difference(
    struct timespec *end,
    const struct timespec *start
) {
    // Calculate difference in seconds
    end->tv_sec -= start->tv_sec;
    
    // Calculate difference in nanoseconds
    if (end->tv_nsec < start->tv_nsec) {
        end->tv_sec -= 1; // Borrow 1 second
        end->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    } else {
        end->tv_nsec -= start->tv_nsec;
    }
}

void main_loop(struct Game * game) {
    // Todo: implement 100% cpu usage loop using `clock_gettime`
    //       check if enough time has passed for an update to happen
    //       possibly sleep for like 2ms so the cpu isn't abused (2ms might in theory be worse)
    //       store time_update in entity, pass `dt` to update (?)
    //                not sure how to handle passed time if entities have different values
    //                possibly store the passed time inside the entity aswell?
    //       :Essaying:
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

        if (end.tv_nsec > 200000000) {
            render_game(game);
            render_game_state (game);
            
            wrefresh(game->info_panel);
            wrefresh(game->window);
            handle_key_down(game, key);
            update_game(game);
            clock_gettime(CLOCK_REALTIME, &start);
            key = ERR;
        }
    } while (!game->over);

    render_game(game);
    render_game_state (game);
}

int main() {
    srand(time(NULL));
    srand(rand());
    srand(rand());
    struct Game game = { 0 };
    init_game(&game);

    WINDOW * main_window = init_curses(&game.config);

    init_sub_windows(&game, main_window);

    main_loop(&game);

    fprintf(stderr, "Game Ended with %d\n", game.over);
    if (game.over == WIN) {
        read_player_name(&game);
    }

    destroy_game(&game);
    endwin();
    
    return 0;
}