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
    Symbol background, background_color;      // Strip can only have one backgroud (more is unnecessary)
    CellType collision_type;
    
    // Movable Strip definition
    int direction;
    int has_random_velocity, velocity;
    struct Entity * removed_entity;
};

typedef struct Strip Strip;
typedef struct Entity Entity;


void render_strip(WINDOW *, Strip *, struct Game *, int);

Strip * _create_strip_common(struct Game *);

int get_strip_index(Strip *, Strip **);

int is_player_near(struct Game *, struct Entity *, int);

void change_random_velocity(struct Game *, struct Entity *);

int update_entity_moveable(Strip *, struct Game *, struct Entity *, int, int);

unsigned get_entity_tail_position(Strip *, struct Entity *, struct Game *);

void try_readd_vehicle(Strip *, struct Game *);

void try_remove_vehicle(Strip *, struct Entity *, struct Game *);

void update_strip_moveable(Strip *, struct Game *);

int is_entity_at(Entity *, unsigned, struct Game *);

void add_entity_at_position(Strip *, Entity *, int);

void add_entity_to_strip(Strip *, Entity *, struct Game *);

int get_strip_index(Strip *, Strip **);

Strip * _create_strip_movable(Symbol, Entity *, size_t, size_t, struct Game *);

Strip * create_strip_river(struct Game *);

Strip * create_strip_road(struct Game *);

Strip * create_strip_forest(struct Game *);

Strip * create_strip_empty(struct Game *);

void free_entity_list(struct Entity *);

void free_strip(Strip *);

#endif
