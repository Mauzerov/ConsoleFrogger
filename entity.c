#include "game.h"
#include "entity.h"

int can_move(struct Entity * e) {
    e->state = (e->state + 1) % e->velocity;
    return e->state == 0;
}

void undo_move(struct Game * game) {
    assert(game->player.x || game->player.y);
    // undo prev move
    game->player.x -= game->prev_move.x;
    game->player.y -= game->prev_move.y;
    // clear prev move
    memset(&game->prev_move, 0, sizeof(struct Point));
}

void end_game(struct Game * game, int reason) {
    game->over = reason;
}

void handle_entity_collision(
    struct Entity * entity,
    struct Game * game
) {
    switch (entity->type) {
    case Solid:
        undo_move(game);
        break;
    case Evil:
        end_game(game, LOSS);
        break;
    default:
        break;
    }
}
