#ifndef STRIP_H
#define STRIP_H

#define UPDATE_LEFT -1
#define UPDATE_RIGHT 1

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cell.h"
#include "game.h"

struct Game;

// NOTE: Entity should overload most (if not all) of Cell
struct Entity {
    Symbol symbol;
    UID uid;
    unsigned width;
    unsigned position;
    struct Entity * next;
};

struct Strip {
    struct Entity * entities; // Linked List
    Symbol bg;
    void (*update)(struct Strip * self, struct Game * game);
    void (*render)(struct Strip * self, struct Game * game);
    int direction;
    int state, velocity;
    int entity_count;
};

typedef struct Strip Strip;
typedef struct Entity Entity;


void render_strip(Strip *, struct Game *);
void render_cell(Symbol, struct Game *);

Strip * _create_strip_common();

void _update_strip_moveable(Strip *, struct Game *);

int entity_fits(Strip *, Entity *, unsigned, Symbol, struct Game *);

int add_entity_at(Strip *, Entity *, Symbol, struct Game *, int);

int add_entity(Strip *, Entity *, Symbol, struct Game *);

Strip * _create_strip_movable(Symbol, Entity *, size_t, size_t, struct Game *);

Strip * create_strip_river(struct Game *);

Strip * create_strip_road(struct Game *);

Strip * create_strip_forest(struct Game *);

Strip * create_strip_empty(struct Game *);

void destroy_strip(Strip *);

#endif 