#ifndef STORK_H
#define STORK_H

#include "game.h"

void init_stork(struct Game *);
void update_stork(struct Game * game);

#ifdef STORK_IMPLEMENTATION

void init_stork(struct Game * game) {
    struct Entity * stork = game->stork = calloc(1, sizeof(Entity));
    if ((stork->pos.x = game->config.STORK_X) < 0)
        stork->pos.x = rand() % game->size.x;
    stork->pos.y = game->size.y / 3;
    stork->velocity = game->config.STORK_VELOCITY;
}

void update_stork(struct Game * game) {
    struct Point delta = { 0 },
               * stork_pos = &game->stork->pos;
    struct Entity * stork = game->stork;
    if (can_move(stork)) {
        delta.x = game->player.x - stork_pos->x;
        delta.y = game->player.y - stork_pos->y;

        stork_pos->y += sgn(delta.y);
        stork_pos->x += sgn(delta.x);
    }
    if (point_overlapping(stork_pos, &game->player)) {
        game->over = LOSS;
    }
}

#endif
#endif