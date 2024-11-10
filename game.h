#ifndef GAME_H
#define GAME_H

#define GAME_WIDTH  20
#define GAME_HEIGHT 10

#define CARS_PER_STRIP  2
#define LOGS_PER_STRIP  2
#define TREES_PER_STRIP 2

#include "strip.h"

struct Point {
    int x, y;
};

struct Game {
    struct Strip ** strips;
    struct Point player,
                 prev_move,
                 size,
                 cursor;
    int over;
};

struct Point get_offset(struct Game *);

void init_game(struct Game *);

void render_border(struct Game *, struct Point);

void render_game(struct Game *);

void handle_collision_postupdate(struct Game *);

void handle_collision_preupdate(struct Game *);

void update_game(struct Game *);

void destroy_game(struct Game *);

#endif