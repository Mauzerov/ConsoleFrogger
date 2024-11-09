#include "engine.h"

#include "strip.h"
#include "game.h"

void init_game(struct Game * game) {
    struct Strip*(*StripConstructors[])(struct Game *) = {
        create_strip_river,
        create_strip_road,
        create_strip_forest,
        _create_strip_common
    };
    size_t STRIP_COUNT = sizeof(StripConstructors) / sizeof(StripConstructors[0]);

    game->strips = malloc(GAME_HEIGHT * sizeof(struct Strip *));
    game->size.x = GAME_WIDTH;
    game->size.y = GAME_HEIGHT;
    for (int i = 0; i < game->size.y; i++) {
        struct Strip * strip = StripConstructors[rand() % STRIP_COUNT](game);
        game->strips[i] = strip;
    }
}

void render_border(struct Game * game) {
    Cell border = { Border };
    game->cursor.x = game->cursor.y = -1;
    for (int i = 0; i <= game->size.x + 1; i++) {
        render_cell(&border, game);
        game->cursor.x++;
    }
    game->cursor.y = game->size.y;
    game->cursor.x = -1;
    for (int i = 0; i <= game->size.x + 1; i++) {
        render_cell(&border, game);
        game->cursor.x++;
    }
}

void render_game(struct Game * game) {
    render_border(game);
    game->cursor.y = 0;
    for (int i = 0; i < game->size.y; i++) {
        invoke(game->strips[i]->render, game->strips[i], game);
    }
    memcpy(&game->cursor, &game->player, sizeof(struct Point));
    Cell frog = (Cell) {
        Frog
    };
    render_cell(&frog, game);
    attron(COLOR_PAIR(Null));
}

void handle_collision_postupdate(struct Game * game) {
    Strip * strip = game->strips[game->player.y];
    Symbol symbol = strip->items [game->player.x].symbol;

    switch (symbol) {
    case Water:
    case Car:
        game->over = 1;
        break;
    default:
        break;
    }
}

void handle_collision_preupdate(struct Game * game) {
    Strip * strip = game->strips[game->player.y];
    Symbol symbol = strip->items [game->player.x].symbol;

    switch (symbol) {
    case Tree:
        assert(game->player.x || game->player.y);
        game->player.x -= game->prev_move.x;
        game->player.y -= game->prev_move.y;
        memset(&game->prev_move, 0, sizeof(struct Point));
        // if player move up/down on a moving strip they can hit a tree
        //    then they remain on the moving item
        handle_collision_preupdate(game);
        break;
    case Log:
    case Taxi:
        game->player.x = (
            game->player.x + strip->direction + game->size.x
        ) % game->size.x;
        break;
    default:
        break;
    }
}

void update_game(struct Game * game) {
    handle_collision_preupdate(game);
    for (int i = 0; i < game->size.y; i++) {
        invoke(game->strips[i]->update, game->strips[i], game);
    }
    handle_collision_postupdate(game);
}

void destroy_game(struct Game * game) {
    for (int i = 0; i < game->size.y; i++) {
        destroy_strip(game->strips[i]);
    }
    free(game->strips);
}