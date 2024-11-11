#ifndef GAME_H
#define GAME_H

#define GAME_WIDTH  20
#define GAME_HEIGHT 10

#define WIN 2
#define LOSS 1

#define LEADERBOARD_FILENAME "frogger.score"
#define LEADERBOARD_SIZE 5

#include "strip.h"

struct Point {
    int x, y;
};

struct Config {
    int CARS_PER_STRIP,
        LOGS_PER_STRIP,
        TREES_PER_STRIP,
        CHANCE_OF_SLOW_STRIP,
        TIMEOUT;
};

struct Game {
    struct Strip ** strips;
    struct Point player,
                 prev_move,
                 size,
                 cursor;
    struct Config config;
    int over;
    unsigned long score;
};

struct Player {
    char name[20];
    unsigned long score;
};
typedef struct Player Player;

struct Point get_offset(struct Game *);

void read_config_file(struct Game *);

void init_game(struct Game *);

void render_border(struct Game *, struct Point);

void render_game(struct Game *);

void handle_collision_postupdate(struct Game *);

void handle_collision_preupdate(struct Game *);

void update_game(struct Game *);

void destroy_game(struct Game *);

#endif