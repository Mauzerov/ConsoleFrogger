#ifndef GAME_H
#define GAME_H

#define MAX_GAME_WIDTH  45
#define MAX_GAME_HEIGHT 100

#define INFO_PANEL_WIDTH  28
#define INFO_PANEL_HEIGHT 15

#define WIN 2
#define LOSS 1

#define LEADERBOARD_FILENAME "frogger.score"
#define LEADERBOARD_SIZE 5

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
        VISIBLE_STRIPS,
        VISIBLE_AHEAD
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
};

int moveby(void *, int, int);

// struct Point get_offset(struct Game *);

void read_config_file(struct Game *);

void init_game(struct Game *);

void render_leaderboard(struct Game *);

void render_game_state(struct Game *);

void render_border(WINDOW *, struct Game *);

void render_game(struct Game *);

unsigned resolve_player_collisions(struct Strip *, struct Game *);

void update_game(struct Game *);

void destroy_game(struct Game *);

#define max(a, b) (((a) < (b)) ? (b) : (a))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define clamp(x_min, x, x_max) ( \
    ((x) < (x_min)) ?            \
        (x_min)     :            \
        (((x) > (x_max)) ?       \
            (x_max)      : (x))  \
)

#endif