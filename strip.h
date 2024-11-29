#ifndef STRIP_H
#define STRIP_H

#define UPDATE_LEFT -1
#define UPDATE_RIGHT 1

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "engine.h"
#include "cell.h"
#include "game.h"
#include "entity.h"

struct Game;

struct Strip {
    struct Entity * entities; // Linked List
    Symbol bg, bg_color;      // Strip can only have one backgroud (more is unnecessary)
    
    void (*update)   (struct Strip *, struct Game *);
    void (*render)   (WINDOW *, struct Strip *, struct Game *, int);
    void (*collide)  (struct Game *);
    // Movable Strip definition
    int direction;
    int has_random_velocity, velocity;
    int entity_count;
};

typedef struct Strip Strip;
typedef struct Entity Entity;


void render_strip(WINDOW *, Strip *, struct Game *, int);
// void render_cell(Symbol, struct Game *);

Strip * _create_strip_common(struct Game *);

void _update_strip_moveable(Strip *, struct Game *);

int is_entity_at(Entity *, unsigned, struct Game *);

int add_entity_at(Strip *, Entity *, struct Game *, int);

int add_entity(Strip *, Entity *, struct Game *);

int get_strip_index(Strip *, Strip **);

Strip * _create_strip_movable(Symbol, Entity *, size_t, size_t, struct Game *);

Strip * create_strip_river(struct Game *);

Strip * create_strip_road(struct Game *);

Strip * create_strip_forest(struct Game *);

Strip * create_strip_empty(struct Game *);

void destroy_strip(Strip *);

#endif 
