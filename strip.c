#include <wchar.h>

#include "engine.h"

#include "strip.h"
#include "cell.h"
#include "game.h"

void render_symbol(Symbol symbol, struct Game * game) {
    static char symbols[]  = "# @&~=_OT";
    attron(COLOR_PAIR(symbol));
    mvaddch(game->cursor.y, game->cursor.x, symbols[symbol]);
}

void render_strip(Strip * self, struct Game * game) {
    int cursor_x = game->cursor.x;
    for (int i = 0; i < game->size.x; i++) {
        render_symbol(self->bg, game);
        game->cursor.x++;
    }
    struct Entity * head = self->entities;
    while (head != NULL) {
        for (unsigned i = 0; i < head->width; i++) {
            game->cursor.x = cursor_x + (head->position + i) % game->size.x;
            render_symbol(head->symbol, game);
        }
        head = head->next;
    }
}

Strip * _create_strip_common(struct Game * game) {
    Strip* self = malloc(sizeof(Strip));
    // self->items = malloc(sizeof(struct Cell) * game->size.x);
    self->entities = NULL; // malloc(sizeof(struct Entity));
    self->render = render_strip;
    self->bg = Null;
    (void)game;

    return self;
}

/**
 *  MM   MM  OOOOO  VV    VV  AA   BBBBB  LL    EEEEEE
 *  MMM MMM OO   OO  VV  VV  AAAA  B   BB LL    EE   
 *  M MMM M OO   OO  VV  VV  A  A  BBBBB  LL    EEEE 
 *  M  M  M OO   OO   VVVV  AAAAAA B   BB LL    EE   
 *  M     M  OOOOO     VV   AA  AA BBBBB  LLLLL EEEEEE
 * 
 **/

void _update_strip_moveable(Strip * self, struct Game * game) {
    assert(self->direction != 0 && "Movable Strip cannot be static!");
    assert(self->velocity && "Movable Strip cannot be static!");
    
    if ((self->state = (self->state + 1) % self->velocity) != 0)
        return;

    struct Entity * head = self->entities;
    while (head != NULL) {
        head->position = (head->position + game->size.x + self->direction) % game->size.x;
        head = head->next;
    }
}

int entity_fits(
    Strip * self, Entity * entity,
    unsigned index, Symbol bg,
    struct Game * game
) {
    (void)self;
    (void)entity;
    (void)index;
    (void)bg;
    (void)game;
    return 1;    
}

int add_entity_at(
    Strip * self,
    Entity * entity,
    Symbol bg,
    struct Game * game,
    int position
) {
    static UID uid = 0; 
    (void)game;
    (void)bg;

    entity->position = position != -1 ? position : rand() % game->size.x;
    entity->uid = ++uid;

    if (self->entities == NULL){
        self->entities = malloc(sizeof(Entity));
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


int add_entity(Strip * self, Entity * entity, Symbol bg, struct Game * game) {
    return add_entity_at(self, entity, bg, game, -1);
}

Strip * _create_strip_movable(
    Symbol bg, Entity * fg,
    size_t n_fg, size_t fg_count,
    struct Game * game
) {
    Strip * self = _create_strip_common(game);

    self->direction = (rand() & 1) ? UPDATE_RIGHT : UPDATE_LEFT;
    self->velocity  = (rand() % game->config.CHANCE_OF_SLOW_STRIP == 0) + 1;
    self->update = _update_strip_moveable;
    self->bg = bg;

    for (size_t i = 0; i < fg_count; i++) {
        Entity entity = fg[rand() % n_fg];
        add_entity(self, &entity, bg, game);
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
        { Log, .width = 2 },
        { Log, .width = 3 },
    };
    return _create_strip_movable(
        Water,
        fg, sizeof(fg) / sizeof (fg[0]) ,
        game->config.LOGS_PER_STRIP,
        game
    );
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
        { Car,  .width = 1 },
        { Car,  .width = 1 },
        { Taxi, .width = 1 }
    };
    return _create_strip_movable(
        Curb,
        fg, sizeof(fg) / sizeof (fg[0]),
        game->config.CARS_PER_STRIP,
        game
    );
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
        { Tree, .width = 1 },
    };
    Strip * self = _create_strip_common(game);

    for (int i = 0 ; i < game->config.TREES_PER_STRIP; i++) {
        add_entity(self, fg, Null, game);
    }
    return self;
}

Strip * create_strip_empty(struct Game * game) {
    return _create_strip_common(game);
}

void destroy_linked_list(struct Entity * entity) {
    if (entity != NULL) {
        destroy_linked_list(entity->next);
        free(entity);
    }
}

void destroy_strip(Strip * self) {
    destroy_linked_list(self->entities);
    // if (self->items != NULL)
    //     free(self->items);
    free(self);
}

