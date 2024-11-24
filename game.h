#ifndef GAME_H
#define GAME_H

#define MAX_GAME_WIDTH  25
#define MAX_GAME_HEIGHT 100

#define CELL_WIDTH 3
#define CELL_HEIGHT 2

#define WIN 2
#define LOSS 1

#define LEADERBOARD_FILENAME "frogger.score"
#define LEADERBOARD_SIZE 5

#define INFO_PANEL_WIDTH  38
#define INFO_PANEL_HEIGHT (LEADERBOARD_SIZE + 2)

#define CONFIG_FILE_NAME "frogger.config"

#include "engine.h"
#include "strip.h"
#include "entity.h"

struct Point {
    int x, y;
};

struct Player {
    char name[20];
    unsigned long score;
};
typedef struct Player Player;

struct Config {
    int CARS_PER_STRIP,
        LOGS_PER_STRIP,
        TREES_PER_STRIP,
        CHANCE_OF_SLOW_STRIP,
        TIMEOUT,
        SEED,
        VISIBLE_STRIPS,
        VISIBLE_AHEAD,
        CHANCE_OF_SPEED_CHANGE
    ;
};

struct Game {
    struct Strip ** strips;
    struct Point player,
                 prev_move,
                 size,
                 cursor;
    struct Config config;
    WINDOW * window,
           * info_panel;
    Player leaderboard[LEADERBOARD_SIZE];
    int player_count;
    int over;
    unsigned long score;
    char textures[Symbol_Count][CELL_WIDTH * CELL_HEIGHT];
    short colors[Symbol_Count][3];
};

int moveby(void *, int, int);

void read_textures(struct Game *, FILE *);

void read_config_file(struct Game *);

void init_game(struct Game *);

void render_leaderboard(struct Game *);

void render_game_state(struct Game *);

void render_border(WINDOW *, struct Game *);

void render_game(struct Game *);

unsigned handle_player_collisions(struct Strip *, struct Game *);

unsigned _P(int, struct Game *);

void update_game(struct Game *);

void destroy_game(struct Game *);

#endif