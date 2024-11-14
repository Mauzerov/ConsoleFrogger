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
    int height = game->size.y + 1;
    int width  = game->size.x + 1;

    for (int y = 0; y <= height; y++) {
        for (int x = 0; x <= width; x++) {
            if (!!(x % width) ^ !!(y % height)) {
                game->cursor.x = off.x - 1 + x;
                game->cursor.y = off.y - 1 + y;
                // render_cell(&border, game);
                render_symbol(Border, game);
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
    render_symbol(Frog, game);
    attron(COLOR_PAIR(Null));
}

void handle_game_over(struct Game * game, CollideFunction death) {
    Strip * strip = game->strips[game->player.y];
    struct Entity * head = strip->entities;
    if (game->player.y == 0) {
        // Win
        game->over = WIN;
        return;
    }   
    unsigned collitions = 0;
    while (head != NULL) {
        // TODO: below is ugly (find a better solution)
        if (is_entity_at(head, game->player.x, game))
        if (head->on_collide == death){
            invoke(head->on_collide, head, game);
            collitions++;
        }
        head = head->next;
    }
    // if (!collitions)
    //     invoke(strip->collide, NULL, game);
}

void handle_entity_collision(struct Game * game, CollideFunction death) {
    Strip * strip = game->strips[game->player.y];
    struct Entity * head = strip->entities;
    unsigned collitions = 0;
    while (head != NULL) {
        if (is_entity_at(head, game->player.x, game)) {
            fprintf(stderr, "Collided at with %d\n", game->player.x);
            if (head->on_collide != death) {
                invoke(head->on_collide, head, game);
                fprintf(stderr, "Collided with %u\n", head->symbol);
                collitions++;
            }    
        }        
        head = head->next;
    }
    if (!collitions){
        invoke(strip->collide, NULL, game);
    }
}

void update_game(struct Game * game) {
    game->score++;
    // non game over
    handle_entity_collision(game, entity_oncollide_death);
    for (int i = 0; i < game->size.y; i++) {
        invoke(game->strips[i]->update, game->strips[i], game);
    }
    // game over 
    handle_game_over(game, entity_oncollide_death);
}

void destroy_game(struct Game * game) {
    for (int i = 0; i < game->size.y; i++) {
        destroy_strip(game->strips[i]);
    }
    free(game->strips);
}