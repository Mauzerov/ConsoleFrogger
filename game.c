#include "engine.h"

#include "strip.h"
#include "game.h"

extern int read_leaderboard(FILE *, Player[LEADERBOARD_SIZE]);

void render_leaderboard(struct Game * game, struct Point off) {
    (void)off; (void)game;
    FILE * file = fopen(LEADERBOARD_FILENAME, "r");

    if (file == NULL)
        return;

    Player leaderboard[LEADERBOARD_SIZE] = { 0 };
    int player_count = read_leaderboard(file, leaderboard);

    int posy = off.y + game->size.y + 2;
    mvaddstr(posy - 1, off.x + game->size.x / 2 - 5, "LEADERBOARD");
    
    char leaderboard_line[40] = { 0 };
    for (int i = 0; i < player_count; i++) {
        int lenght = sprintf(leaderboard_line, "%d. %-20s: %05lu", i + 1,
            leaderboard[i].name, leaderboard[i].score);
        int posx = off.x + ((game->size.x - lenght) >> 1);
        mvaddstr(posy + i, posx, leaderboard_line);
    }
}

#define ConfigRead(config, field)          \
    if (strcmp(#field, buffer) == 0) {     \
        fscanf(file, "%d", &config.field); \
    }
#define CONFIG_FILE_NAME "frogger.config"

void read_config_file(struct Game * game) {
    FILE * file = fopen(CONFIG_FILE_NAME, "r");
    if (file == NULL)
        return;

    char buffer[20] = { 0 };
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
        ConfigRead(game->config, TIMEOUT);
    }
    fclose(file);
}

void init_strips(struct Game * game) {
    struct Strip*(*StripConstructors[])(struct Game *) = {
        create_strip_river,
        create_strip_road,
        create_strip_road,
        create_strip_forest,
        create_strip_empty
    };
    size_t STRIP_COUNT = sizeof(StripConstructors) / sizeof(StripConstructors[0]);

    int prev_direction = 0;
    for (int i = 0; i < game->size.y; i++) {
        if (i % (game->size.y - 1) == 0) {
            game->strips[i] = create_strip_empty(game);
            continue;
        }
        struct Strip * strip = StripConstructors[rand() % STRIP_COUNT](game);

        // Force Movable Strip to be in opposite directions
        if (strip->direction == prev_direction) {
            strip->direction *= -1;
        }
        prev_direction = strip->direction;
        game->strips[i] = strip;
    }
}

void init_game(struct Game * game) {
    game->size.x = GAME_WIDTH;
    game->size.y = GAME_HEIGHT;

    read_config_file(game);
    game->player.y = game->size.y - 1;

    game->strips = malloc(game->size.y * sizeof(struct Strip *));

    init_strips(game);
}

struct Point get_offset(struct Game * game) {
    int offx, offy;
    getmaxyx(stdscr, offy, offx);

    return (struct Point) {
        .x = (offx - game->size.x) / 2,
        .y = (offy - game->size.y) / 2 - 2,
    };
}

void render_game_state(struct Game * game, struct Point off) {
    char buffer[100] = { 0 };
    int lenght = sprintf(buffer, "Score: %05lu", game->score);
    mvaddstr(off.y - 2, off.x + ((game->size.x - lenght) >> 1), buffer);
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
    render_game_state(game, off);
    render_leaderboard(game, off);

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

    if (game->player.y == 0) {
        // Win
        game->over = WIN;
        return;
    }

    switch (symbol) {
    case Water:
    case Car:
        // Loss
        game->over = LOSS;
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

void update_game(struct Game * game) {
    game->score++;
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