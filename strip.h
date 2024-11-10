#ifndef STRIP_H
#define STRIP_H

#define UPDATE_LEFT -1
#define UPDATE_RIGHT 1

#define CHANCE_OF_SLOW_STRIP 4

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cell.h"
#include "game.h"

struct Game;
    
struct Entity {
    Symbol symbol;
    unsigned width;
};

struct Strip {
    struct Cell * items;
    void (*update)(struct Strip * self, struct Game * game);
    void (*render)(struct Strip * self, struct Game * game);
    int direction;
    int state;
    int velocity;
};

typedef struct Strip Strip;
typedef struct Entity Entity;


void render_strip(Strip *, struct Game *);

Strip * _create_strip_common();

void _update_strip_moveable(Strip *, struct Game *);

void fill_strip(Strip *, Symbol, struct Game *);

int entity_fits(Strip *, Entity *, unsigned, Symbol, struct Game *);

void add_entity(Strip *, Entity *, Symbol, struct Game *);

Strip * _create_strip_movable(Symbol, Entity *, size_t, size_t, struct Game *);

Strip * create_strip_river(struct Game *);

Strip * create_strip_road(struct Game *);

Strip * create_strip_forest(struct Game *);

void destroy_strip(Strip *);
#endif 