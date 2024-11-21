#include "engine.h"

#include "strip.h"
#include "game.h"

unsigned _P(int point, struct Game * game) {
    while (point < 0)
        point += game->size.x;
    return point % game->size.x;
}

extern int read_leaderboard(Player[LEADERBOARD_SIZE]);

void render_leaderboard(struct Game * game) {
    WINDOW * window = game->info_panel;
    Player * leaderboard = game->leaderboard;

    mvwaddstr(window, 0, (INFO_PANEL_WIDTH - 12) >> 1, " SCOREBOARD ");
    
    char leaderboard_line[INFO_PANEL_WIDTH * 2] = { 0 };
    for (int i = 0; i < game->player_count; i++) {
        sprintf(
            leaderboard_line,
            " %1d: %-20s  : %05lu",
            i + 1, leaderboard[i].name, leaderboard[i].score
        );
        mvwaddstr(window, 1 + i, 2, leaderboard_line);
    }
}

void render_game_state(struct Game * game) {
    WINDOW * window = game->window;
    char buffer[100] = { 0 };
    int lenght = sprintf(buffer, " Score: %05lu ", game->score);
    mvwaddstr(window, 0, ((game->size.x * CELL_WIDTH - lenght) >> 1), buffer);
}

#define ConfigRead(config, field)          \
    if (strcmp(#field, buffer) == 0) {     \
        fscanf(file, "%d", &config.field); \
    }

void read_textures(struct Game * game, FILE * file) {
    const int size = CELL_WIDTH * CELL_HEIGHT;
    char c;
    for (int i = 1; i < Symbol_Count; i++) {
        int lenght = 0;
        while (lenght < size) {
            fscanf(file, "%c", &c);
            if (c == '\n') continue;
            game->textures[i][lenght++] = c;
        }
    }
}

void read_config_file(struct Game * game) {
    FILE * file = fopen(CONFIG_FILE_NAME, "r");
    if (file == NULL)
        return;

    char buffer[41] = { 0 };
    while (fscanf(file, "%40s", buffer) == 1) {
        if (strcmp(buffer, "BOARD_SIZE") == 0) {
            fscanf(file, "%d %d", &game->size.x, &game->size.y);
        }
        if (strcmp(buffer, "PLAYER_X") == 0) {
            fscanf(file, "%d", &game->player.x);
        }
        if (strcmp(buffer, "TEXTURES") == 0) {
            read_textures(game, file);
        }
        ConfigRead(game->config, CARS_PER_STRIP);
        ConfigRead(game->config, LOGS_PER_STRIP);
        ConfigRead(game->config, TREES_PER_STRIP);
        ConfigRead(game->config, CHANCE_OF_SLOW_STRIP);
        ConfigRead(game->config, TIMEOUT);
        ConfigRead(game->config, VISIBLE_STRIPS);
        ConfigRead(game->config, VISIBLE_AHEAD);
    }
    fclose(file);
}

int moveby(void * _pos, int by, int size) {
    int * pos = (int*)_pos; // cba with this warning
    *pos = (*pos + by + size) % size;
    return *pos;
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
        // Todo: check is there exists at least one path between trees
        game->strips[i] = strip;
    }
}

void init_game(struct Game * game) {
    read_config_file(game);

    game->player_count = read_leaderboard(game->leaderboard);

    if (game->size.x == 0 || game->size.x > MAX_GAME_WIDTH) {
        // Todo: add error message
        game->size.x = MAX_GAME_WIDTH;
    }
    if (game->size.y == 0 || game->size.y > MAX_GAME_HEIGHT) {
        game->size.y = MAX_GAME_HEIGHT;
    }

    game->player.y = game->size.y - 1;

    game->strips = malloc(game->size.y * sizeof(struct Strip *));

    init_strips(game);
}

struct Point get_offset(struct Game * game) {
    int offx, offy;
    getmaxyx(stdscr, offy, offx);

    return (struct Point) {
        .x = (offx - (game->size.x + 2)) / 2,
        .y = (offy - (game->size.y + 2)) / 2 - 2,
    };
}

void render_border(WINDOW * window, struct Game * game) {
    int height = game->size.y + 1;
    int width  = game->size.x + 1;

    for (int y = 0; y <= height; y++) {
        for (int x = 0; x <= width; x++) {
            if (!!(x % width) ^ !!(y % height)) {
                game->cursor.x = 1 + x;
                game->cursor.y = 1 + y;
                render_symbol(window, Border, game);
            }
        }
    }
}

void render_game(struct Game * game) {
    WINDOW * window = game->window;

    int visibility = game->config.VISIBLE_STRIPS;
    int scroll = clamp(
        0, game->player.y - game->config.VISIBLE_AHEAD, game->size.y - visibility
    );

    for (int i = 0; i < visibility; i++) {
        game->cursor.y = i;
        game->cursor.x = 0;
        invoke(
            game->strips[i + scroll]->render,
            window, game->strips[i + scroll], game
        );
    }

    game->cursor.x = game->player.x;
    game->cursor.y = game->player.y - scroll;
    render_symbol(window, Frog, game);
    attron(COLOR_PAIR(Null));
}

unsigned resolve_player_collisions(Strip * strip, struct Game * game) {
    struct Entity * head = strip->entities;
    unsigned collitions = 0;
    while (head != NULL) {
        if (is_entity_at(head, game->player.x, game)) {
            handle_entity_collision(head, game);
            collitions++;
        }        
        head = head->next;
    }
    return collitions;
}

void update_game(struct Game * game) {
    Strip * playerStrip = game->strips[game->player.y];
    game->score++;

    if (game->player.y == 0) {
        end_game(game, WIN);
        return; // if player won no need to check collisions
    }
    
    if (resolve_player_collisions(playerStrip, game) == 0u)
        invoke(playerStrip->collide, game);

    for (int i = 0; i < game->size.y; i++) {
        invoke(game->strips[i]->update, game->strips[i], game);
    }
    // second call required, so that the fail screen isn't awkward
    if (resolve_player_collisions(playerStrip, game) == 0u)
        invoke(playerStrip->collide, game);
}

void destroy_game(struct Game * game) {
    for (int i = 0; i < game->size.y; i++) {
        destroy_strip(game->strips[i]);
    }
    free(game->strips);
}