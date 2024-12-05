#include "engine.h"

#include "strip.h"
#include "cell.h"
#include "game.h"
#include "entity.h"

Strip * _create_strip_common(struct Game * game) {
    Strip* self = calloc(1, sizeof(Strip));

    self->bg = Null;
    self->bg_color = Null;
    (void)game;
    return self;
}

int get_strip_index(Strip * self, Strip ** strips) {
    int strip_y = 0;
    while (strips[strip_y] != self) strip_y++;
        return strip_y;
    assert(0 && "unreachable");
}

int is_player_near(struct Game * game, struct Entity * head, int entity_y) {
    // Check vertical proximity
    int vertical_proximity = abs(game->player.y - entity_y) <= 1;

    // Check horizontal proximity with wrap-around
    int horizontal_distance = abs(game->player.x - head->pos.x);
    int wrap_around_distance = game->size.x - horizontal_distance;
    int horizontal_proximity = (horizontal_distance <= 2) || (wrap_around_distance <= 2);

    return vertical_proximity && horizontal_proximity;
}
/**
 *  MM   MM  OOOOO  VV    VV  AA   BBBBB  LL    EEEEEE
 *  MMM MMM OO   OO  VV  VV  AAAA  B   BB LL    EE   
 *  M MMM M OO   OO  VV  VV  A  A  BBBBB  LL    EEEE 
 *  M  M  M OO   OO   VVVV  AAAAAA B   BB LL    EE   
 *  M     M  OOOOO     VV   AA  AA BBBBB  LLLLL EEEEEE
 * 
 **/
void change_random_velocity(struct Game * game, struct Entity * head) {
    if (random_chance(game->config.CHANCE_OF_SPEED_CHANGE)) {
        head->velocity = (head->velocity == game->config.SLOW_VELOCITY)
                       ? game->config.NORMAL_VELOCITY : game->config.SLOW_VELOCITY;
    }
}

int update_entity_moveable(
    Strip * self,
    struct Game * game,
    struct Entity * head,
    int entity_y,
    int player_moved
) {
    if (self->has_random_velocity)
        change_random_velocity(game, head);

    int can_travel = game->willing_to_travel || head->symbol == Log;
    if (!head->stop_when_player_near || !is_player_near(game, head, entity_y)) {
        // and move player along if wasn't moved already
        if (game->player.y == entity_y &&
            !player_moved &&
            can_travel    &&
            is_entity_at(head, game->player.x, game)
        ) {
            moveby(&game->player.x, self->direction, game->size.x);
            player_moved = TRUE;
        }
        moveby(&head->pos.x, self->direction, game->size.x);
    } else {
        // player is near a car -> don't move the car
    }
    return player_moved;
}

unsigned get_entity_tail_position(
    Strip * self,
    struct Entity * entity,
    struct Game * game
) {
    if (self->direction < 0)
        return (entity->pos.x + entity->width) % game->size.x;
    return entity->pos.x;
}

void try_readd_car(
    Strip * self,
    struct Game * game
) {
    if (!self->entity_in_gulag)
        return;
    if (!random_chance(game->config.CHANCE_OF_CAR_DEATH)) 
        return;
    self->entity_in_gulag->next = NULL;
    add_entity_at(self, self->entity_in_gulag, game, 0);
    free(self->entity_in_gulag);
    self->entity_in_gulag = NULL;
}

void try_remove_car(
    Strip * self,
    struct Entity * entity,
    struct Game * game
) {
    if (self->entity_in_gulag)
        return;
    if (get_entity_tail_position(self, entity, game) != 0u)
        return;
    if (!random_chance(game->config.CHANCE_OF_CAR_DEATH)) 
        return;

    assert(entity != NULL);
    self->entity_in_gulag = entity;

    struct Entity * head = self->entities;
    if (self->entities == entity)
        self->entities = entity->next;
    else while (head != NULL) {
        if (head->next == entity) {
            head->next = entity->next;
            break;
        }
        head = head->next;
    }
}


void update_strip_moveable(Strip * self, struct Game * game) {
    assert(self->direction != 0 && self->velocity &&
            "Movable Strip cannot be static!");

    int strip_y = get_strip_index(self, game->strips);
    int player_moved = FALSE;
    // if strip should be moved; move all its entities
    struct Entity * head = self->entities;
    while (head != NULL) {
        if (can_move(head)){
            player_moved |= update_entity_moveable(
                self, game,
                head,
                strip_y, player_moved
            );
            if (head->symbol != Log){
                try_remove_car(self, head, game);
                try_readd_car(self,  game);
            }
        }
        head = head->next;
    }
}

/*
 * IIII NN   N IIII TTTTTT 
 *  II  NNN  N  II  TTTTTT 
 *  II  N NN N  II    TT   
 *  II  N  NNN  II    TT   
 * IIII N   NN IIII   TT 
 */

int is_entity_at(Entity * entity, unsigned index, struct Game * game) {
    unsigned start = entity->pos.x,
             end   = (start + entity->width) % game->size.x;

    if (start <= end) {
        return start <= index && index < end;
    }
    // Handle wraparound
    return start <= index || index < end;
}

void add_entity_at(
    Strip * self,
    Entity * entity,
    struct Game * game,
    int position
) {
    entity->pos.x = (position >= 0) ? position : rand() % game->size.x;
    entity->velocity = self->velocity;

    if (self->entities == NULL){
        self->entities = calloc(1, sizeof(Entity));
        memcpy(self->entities, entity, sizeof(Entity));
        return;
    }

    Entity * head = self->entities;
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = malloc(sizeof(Entity));
    memcpy(head->next, entity, sizeof(Entity));
}


void add_entity(Strip * self, Entity * entity, struct Game * game) {
    add_entity_at(self, entity, game, -1);
}

Strip * _create_strip_movable(
    Symbol bg, 
    Entity * fg, size_t n_fg, size_t fg_count,
    struct Game * game
) {
    Strip * self = _create_strip_common(game);

    self->direction = random_chance(50) ? UPDATE_RIGHT : UPDATE_LEFT;
    self->velocity  = random_chance(game->config.CHANCE_OF_SLOW_STRIP)
                    ? game->config.SLOW_VELOCITY : game->config.NORMAL_VELOCITY;
    self->bg = bg;

    for (size_t i = 0; i < fg_count; i++) {
        Entity entity = fg[rand() % n_fg];
        add_entity(self, &entity, game);
    }
    self->entity_in_gulag = NULL;
    return self;
}

/**
 *   RRRRR   II VV    VV EEEEE  RRRRR 
 *   RR  RR  II  VV  VV  EE     RR  RR
 *   RRRRR   II  VV  VV  EEEE   RRRRR 
 *   RR  RR  II   VVVV   EE     RR  RR
 *   RR  RR  II    VV    EEEEE  RR  RR
 * 
 **/

Strip * create_strip_river(struct Game * game) {
    Entity fg[] = {
        { Log, .width = 2, },
        { Log, .width = 3, },
    };

    Strip * self = _create_strip_movable(
        Water,
        fg, sizeof(fg) / sizeof (fg[0]) ,
        game->config.LOGS_PER_STRIP,
        game
    );
    if (can_change_color()) {
        const short * bg_color = game->colors[Water];
        self->bg_color = define_new_color(bg_color[0] * .55, bg_color[1]* .55, bg_color[2]* .55);
    } else {
        self->bg_color = COLOR_BLUE;
    }
    self->collide = Evil;
    return self;
}


/**
 *   RRRRR    OOOOO     AA    OOOOOO 
 *   RR  RR  OO   OO   AAAA   OO   OO
 *   RRRRR   OO   OO   A  A   OO   OO
 *   RR  RR  OO   OO  AAAAAA  OO   OO
 *   RR  RR   OOOOO   AA  AA  OOOOOO 
 * 
 **/

Strip * create_strip_road(struct Game * game) {
    Entity fg[] = {
        { Car,  .width = 1, .type = Evil },
        { Car,  .width = 1, .type = Evil,
                .stop_when_player_near = TRUE },
        { Taxi, .width = 1 }
    };
    Strip * self = _create_strip_movable(
        Curb,
        fg, sizeof(fg) / sizeof (fg[0]),
        game->config.CARS_PER_STRIP,
        game
    );
    self->has_random_velocity = TRUE;
    return self;
}

/** 
 *    SSSS TTTTTT  AA  TTTTTT II  CCCC 
 *   SS    ^^TT^^ AAAA ^^TT^^ II CC  CC
 *    SSS    TT   A  A   TT   II CC    
 *      SS   TT  AAAAAA  TT   II CC  CC
 *   SSSS    TT  AA  AA  TT   II  CCCC 
 * 
 **/

Strip * create_strip_forest(struct Game * game) {
    Entity fg[] = {
        { Tree, .width = 1, .type = Solid },
    };
    Strip * self = _create_strip_common(game);

    for (int i = 0 ; i < game->config.TREES_PER_STRIP; i++) {
        add_entity(self, fg, game);
    }
    return self;
}

Strip * create_strip_empty(struct Game * game) {
    return _create_strip_common(game);
}

/**
 * DDDDD  EEEEE  SSSSS TTTTTT RRRRR  UU  UU  CCCC  TTTTTT 
 * DD  DD EE    SS     TTTTTT RR  RR UU  UU CC  CC TTTTTT 
 * DD  DD EEEE   SSSS    TT   RRRRR  UU  UU CC       TT   
 * DD  DD EE        SS   TT   RR  RR UUUUUU CC  CC   TT   
 * DDDDD  EEEEE SSSSS    TT   RR  RR  UUUU   CCCC    TT  
 **/

void destroy_linked_list(struct Entity * entity) {
    if (entity != NULL) {
        destroy_linked_list(entity->next);
        free(entity);
    }
}

void destroy_strip(Strip * self) {
    destroy_linked_list(self->entities);
    if (self->entity_in_gulag)
        free(self->entity_in_gulag);
    free(self);
}

