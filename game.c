#include "engine.h"

#include "strip.h"
#include "game.h"

#define ConfigRead(config, field)          \
    if (strcmp(#field, buffer) == 0) {     \
        fscanf(file, "%d", &config.field); \
    }                                                   
#define CONFIG_FILE_NAME "frogger.config"

void read_config_file(struct Game * game) {
    FILE * file = fopen(CONFIG_FILE_NAME, "r");

    char buffer[20] = { 0 };

    if (file == NULL)
        return;

    while (fscanf(file, "%s", buffer) == 1) {
        if (strcmp(buffer, "BOARD_SIZE") == 0) {
            fscanf(file, "%d %d", &game->size.x, &game->size.y);
        }
        if (strcmp(buffer, "PLAYER_X") == 0) {
            fscanf(file, "%d", &game->player.x);
        }
        ConfigRead(game->config, CARS_PER_STRIP);
        ConfigRead(game->config, LOGS_PER_STRIP);
        ConfigRead(game->config, TREES_PER_STRIP);
        ConfigRead(game->config, CHANCE_OF_SLOW_STRIP);
    }

    fprintf(stderr, "%d\n", game->config.CARS_PER_STRIP);
    fclose(file);
}

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

    game->player.y = game->size.y - 1;
    read_config_file(game);

    int prev_direction = 0;
    for (int i = 0; i < game->size.y - 1; i++) {
        struct Strip * strip = StripConstructors[rand() % STRIP_COUNT](game);

        // Force Movable Strip to be in opposite directions
        if (strip->direction == prev_direction) {
            strip->direction *= -1;
        }
        prev_direction = strip->direction;
        game->strips[i] = strip;
    }
    game->strips[game->size.y - 1] = create_strip_empty(game);
}

struct Point get_offset(struct Game * game) {
    int offx, offy;
    getmaxyx(stdscr, offy, offx);
    offy = (offy - game->size.y) / 2;
    offx = (offx - game->size.x) / 2;
    fprintf(stderr, "%d %d\n", offy, offx);
    return (struct Point) { .x = offx, .y = offy };
}

void render_border(struct Game * game, struct Point off) {
    Cell border = { Border };
    int height = game->size.y + 1;
    int width  = game->size.x + 1;

    for (int y = 0; y <= height; y++) {
        for (int x = 0; x <= width; x++) {
            if (!!(x % width) ^ !!(y % height)) {
                game->cursor.x = off.x - 1 + x;
                game->cursor.y = off.y - 1 + y;
                render_cell(&border, game);
            }
        }
    }
}

void render_game(struct Game * game) {
    struct Point off = get_offset(game);
    render_border(game, off);

    game->cursor.y = off.y;
    for (int i = 0; i < game->size.y; i++) {
        game->cursor.x = off.x;
        invoke(game->strips[i]->render, game->strips[i], game);
        game->cursor.y++;
    }

    game->cursor.x = game->player.x + off.x;
    game->cursor.y = game->player.y + off.y;
    Cell frog = (Cell) { .symbol = Frog };
    render_cell(&frog, game);
    attron(COLOR_PAIR(Null));
}

void handle_collision_postupdate(struct Game * game) {
    Strip * strip = game->strips[game->player.y];
    Symbol symbol = strip->items[game->player.x].symbol;

    switch (symbol) {
    case Water:
    case Car:
        game->over = 1;
        break;
    case Log:
    case Taxi:
        if (strip->state != 0)
            break;
        game->player.x = (
            game->player.x + strip->direction + game->size.x
        ) % game->size.x;
        break;
    default:
        break;
    }
}

void handle_collision_preupdate(struct Game * game) {
    Strip * strip = game->strips[game->player.y];
    Symbol symbol = strip->items[game->player.x].symbol;

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