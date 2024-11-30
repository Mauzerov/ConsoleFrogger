#include <wchar.h>

#include "engine.h"

#include "strip.h"
#include "cell.h"
#include "game.h"
#include "entity.h"

Strip * _create_strip_common(struct Game * game) {
    Strip* self = calloc(1, sizeof(Strip));
    // self->render = render_strip;
    self->bg = Null;
    self->bg_color = Null;
    (void)game;
    return self;
}

int get_strip_index(Strip * self, Strip ** strips) {
    int strip_y = 0;
    while (strips[strip_y] != self) strip_y++;
        return strip_y;
}

/**
 *  MM   MM  OOOOO  VV    VV  AA   BBBBB  LL    EEEEEE
 *  MMM MMM OO   OO  VV  VV  AAAA  B   BB LL    EE   
 *  M MMM M OO   OO  VV  VV  A  A  BBBBB  LL    EEEE 
 *  M  M  M OO   OO   VVVV  AAAAAA B   BB LL    EE   
 *  M     M  OOOOO     VV   AA  AA BBBBB  LLLLL EEEEEE
 * 
 **/

void _update_entity_moveable(
    Strip * self,
    struct Game * game,
    struct Entity * head,
    int entity_y
) {
    if (!can_move(head))
        return;
    if (self->has_random_velocity
        && rand() % 100 < game->config.CHANCE_OF_SPEED_CHANGE) {
        head->velocity = 1 + (head->velocity != 2);
    }

    int player_moved = game->strips[game->player.y] != self;
    // TODO: use _P (possibly rename)
    int player_near = (abs(game->player.y - entity_y) <= 1)
                    && abs(((int)game->player.x - (int)head->pos.x + game->size.x) % game->size.x) <= 2;
    if (head->player_near == NULL || !player_near) {
        // and move player along if wasn't moved already
        if (!player_moved && is_entity_at(head, game->player.x, game)) {
            moveby(&game->player.x, self->direction, game->size.x);
            player_moved = 1;
        }
        moveby(&head->pos.x, self->direction, game->size.x);
    } else {
        head->player_near(head);
    }
}

void update_strip_moveable(Strip * self, struct Game * game) {
    assert(self->direction != 0 && self->velocity &&
            "Movable Strip cannot be static!");
    
    // if ((self->state = (self->state + 1) % self->velocity) != 0)
    //     return;

    int strip_y = get_strip_index(self, game->strips);
    // if strip should be moved; move all its entities
    struct Entity * head = self->entities;
    while (head != NULL) {
        _update_entity_moveable(self, game, head, strip_y);
        head = head->next;
    }

}

int is_entity_at(Entity * entity, unsigned index, struct Game * game) {
    // Todo: Validate this logic
    // maybe convert it to math (2 conditions)
    // checking if a point is in a range that is periodic is weird
    for (unsigned i = 0; i < entity->width; i++) {
        if ((entity->pos.x + i) % game->size.x == index)
            return 1;
    }
    return 0;
}

int add_entity_at(
    Strip * self,
    Entity * entity,
    struct Game * game,
    int position
) {
    entity->pos.x = position != -1 ? position : rand() % game->size.x;
    entity->velocity = self->velocity;

    if (self->entities == NULL){
        self->entities = calloc(1, sizeof(Entity));
        memcpy(self->entities, entity, sizeof(Entity));
        return 1;
    }

    Entity * head = self->entities;
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = malloc(sizeof(Entity));
    memcpy(head->next, entity, sizeof(Entity));
    
    return 1;
}


int add_entity(Strip * self, Entity * entity, struct Game * game) {
    return add_entity_at(self, entity, game, -1);
}

Strip * _create_strip_movable(
    Symbol bg, 
    Entity * fg, size_t n_fg, size_t fg_count,
    struct Game * game
) {
    Strip * self = _create_strip_common(game);

    self->direction = (rand() & 1) ? UPDATE_RIGHT : UPDATE_LEFT;
    self->velocity  = ((rand() % 100) < game->config.CHANCE_OF_SLOW_STRIP) + 1;
    self->bg = bg;

    for (size_t i = 0; i < fg_count; i++) {
        Entity entity = fg[rand() % n_fg];
        add_entity(self, &entity, game);
    }
    self->entity_count = fg_count;
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
    self->collide = lose_game;
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
                .player_near = player_near_car },
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
    free(self);
}

