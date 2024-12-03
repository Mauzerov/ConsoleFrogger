#include <assert.h>

#include "engine.h"

#include "strip.h"
#include "game.h"
#include "stork.h"

unsigned _P(int point, struct Game * game) {
    while (point < 0)
        point += game->size.x;
    return point % game->size.x;
}


extern int read_leaderboard(Player[LEADERBOARD_SIZE]);
extern void render_leaderboard(struct Game * game);

/**
 *  CCCC   OOOOO  NN   N EEEEE IIII  GGGGG 
 * CC  CC OO   OO NNN  N EE     II  GG     
 * CC     OO   OO N NN N EEEE   II  GG GGG 
 * CC  CC OO   OO N  NNN EE     II  GG  GG 
 *  CCCC   OOOOO  N   NN EE    IIII  GGGG  
 **/

#define ConfigRead(config, field)           \
    if (strcmp(#field, buffer) == 0) {      \
        fscanf(file, "%d", &config->field); \
    }

void read_colors(struct Game * game, FILE * file) {
    for (int i = 0; i < Symbol_Count; i++) {
        short r, g, b;
        fscanf(file, " rgb( %hd , %hd , %hd )", &r, &g, &b);
        game->colors[i][0] = linear_scale_rgb(r);
        game->colors[i][1] = linear_scale_rgb(g);
        game->colors[i][2] = linear_scale_rgb(b);
        LOG("curses_rgb(%hd %hd %hd)", game->colors[i][0], game->colors[i][1], game->colors[i][2]);
    }
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

void read_game_config(FILE * file, struct Config * config, char * buffer) {
    ConfigRead(config, CARS_PER_STRIP);
    ConfigRead(config, LOGS_PER_STRIP);
    ConfigRead(config, TREES_PER_STRIP);
    ConfigRead(config, CHANCE_OF_SLOW_STRIP);
    ConfigRead(config, TIMEOUT);
    ConfigRead(config, VISIBLE_STRIPS);
    ConfigRead(config, VISIBLE_AHEAD);
    ConfigRead(config, CHANCE_OF_SPEED_CHANGE);
    ConfigRead(config, CHANCE_OF_CAR_DEATH);
    ConfigRead(config, SEED);
    ConfigRead(config, LEVEL_COUNT);
    ConfigRead(config, STORK_VELOCITY);
    ConfigRead(config, STORK_X);
}

void read_config_file(struct Game * game) {
    FILE * file = fopen(CONFIG_FILE_NAME, "r");
    if (file == NULL)
        return;

    char buffer[41] = { 0 };
    while (fscanf(file, "%40s", buffer) == 1) {
        if (strcmp(buffer, "BOARD_SIZE") == 0) {
            fscanf(file, "%d %d", &game->size.x, &game->size.y);
        } else
        if (strcmp(buffer, "PLAYER_X") == 0) {
            fscanf(file, "%d", &game->player.x);
            if (game->player.x < 0)
                game->player.x = rand() % game->size.x;
        } else
        if (strcmp(buffer, "TEXTURES") == 0) {
            read_textures(game, file);
        } else
        if (strcmp(buffer, "COLORS") == 0) {
            read_colors(game, file);
        } else
        read_game_config(file, &game->config, buffer);
    }
    fclose(file);
}

int moveby(void * _pos, int by, int size) {
    int * pos = (int*)_pos; // cba with this warning
    *pos = (*pos + by + size) % size;
    return *pos;
}

/**
 *  IIII NN   N IIII TTTTTT 
 *   II  NNN  N  II  TTTTTT 
 *   II  N NN N  II    TT   
 *   II  N  NNN  II    TT   
 *  IIII N   NN IIII   TT 
 **/

void init_strips(struct Game * game) {
    struct Strip*(*StripConstructors[])(struct Game *) = {
        create_strip_river,
        create_strip_road,
        create_strip_road,
        create_strip_forest,
        create_strip_empty
    };
    const size_t STRIP_COUNT = sizeof(StripConstructors) / sizeof(StripConstructors[0]);

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
    game->leaderboard_count = read_leaderboard(game->leaderboard);

    if (game->size.x == 0 || game->size.x > MAX_GAME_WIDTH) {
        WARN("Game size exceeded maximum width %d > %d.", game->size.x, MAX_GAME_WIDTH);
        game->size.x = MAX_GAME_WIDTH;
    }
    if (game->size.y == 0 || game->size.y > MAX_GAME_HEIGHT) {
        WARN("Game size exceeded maximum height %d > %d.", game->size.y, MAX_GAME_HEIGHT);
        game->size.y = MAX_GAME_HEIGHT;
    }

    game->player.y = game->size.y - 1;
    game->strips = calloc(game->size.y, sizeof(struct Strip *));

    init_strips(game);
    init_stork(game);
}

/**
 * UU  UU PPPPP  DDDDD    AA   TTTTTT EEEEE 
 * UU  UU PP  PP DD  DD  AAAA  TTTTTT EE    
 * UU  UU PPPPP  DD  DD  A  A    TT   EEEE  
 * UUUUUU PP     DD  DD AAAAAA   TT   EE    
 *  UUUU  PP     DDDDD  AA  AA   TT   EEEEE
 **/

unsigned handle_player_collisions(Strip * strip, struct Game * game) {
    struct Entity * head = strip->entities;
    unsigned collisions = 0;
    while (head != NULL) {
        if (is_entity_at(head, game->player.x, game)) {
            handle_entity_collision(head, game);
            collisions++;
        }
        head = head->next;
    }
    return collisions;
}

void update_game(struct Game * game) {
    Strip * playerStrip = game->strips[game->player.y];
    game->score++; // TODO: update score handling

    if (game->player.y == 0) {
        end_game(game, WIN);
        return; // if player won no need to check collisions
    }
    
    if (handle_player_collisions(playerStrip, game) == 0u
        && playerStrip->collide == Evil)
        end_game(game, LOSS);

    for (int i = 0; i < game->size.y; i++) {
        if (game->strips[i]->velocity != 0)
            update_strip_moveable(game->strips[i], game);
    }
    // second call required, so that the fail screen isn't awkward
    if (handle_player_collisions(playerStrip, game) == 0u
        && playerStrip->collide == Evil)
        end_game(game, LOSS);

    update_stork(game);
}

/**
 * DDDDD  EEEEE  SSSSS TTTTTT RRRRR  UU  UU  CCCC  TTTTTT 
 * DD  DD EE    SS     TTTTTT RR  RR UU  UU CC  CC TTTTTT 
 * DD  DD EEEE   SSSS    TT   RRRRR  UU  UU CC       TT   
 * DD  DD EE        SS   TT   RR  RR UUUUUU CC  CC   TT   
 * DDDDD  EEEEE SSSSS    TT   RR  RR  UUUU   CCCC    TT  
 **/

void destroy_game(struct Game * game) {
    for (int i = 0; i < game->size.y; i++) {
        destroy_strip(game->strips[i]);
    }
    free(game->stork);
    free(game->strips);
}