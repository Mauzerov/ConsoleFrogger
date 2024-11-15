#include "game.h"
#include "entity.h"

typedef struct Entity Entity;

void undo_move(struct Game * game) {
    assert(game->player.x || game->player.y);
    // undo prev move
    game->player.x -= game->prev_move.x;
    game->player.y -= game->prev_move.y;
    // clear prev move
    memset(&game->prev_move, 0, sizeof(struct Point));
    // if player move up/down on a moving strip they can hit a tree
    //    then they remain on the moving item
    // handle_entity_collision(game, entity_oncollide_death);
}

void lose_game(struct Game * game) {
    end_game(game, LOSS);
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
        lose_game(game);
        break;
    default:
        break;
    }
}