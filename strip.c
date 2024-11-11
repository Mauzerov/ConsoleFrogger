#include <wchar.h>

#include "engine.h"

#include "strip.h"
#include "cell.h"
#include "game.h"


void render_cell (Cell  * cell, struct Game * game) {
    static char symbols[]  = "# @&~=_OT";
    attron(COLOR_PAIR(cell->symbol));
    mvaddch(game->cursor.y, game->cursor.x, symbols[cell->symbol]);
}

void render_strip(Strip * self, struct Game * game) {
    for (int i = 0; i < game->size.x; i++) {
        render_cell(&self->items[i], game);
        game->cursor.x++;
    }
}

void fill_strip(Strip * self, Symbol with, struct Game * game) {
    self->bg = with;
    for (int i = 0; i < game->size.x; i++) {
        self->items[i].symbol = with;
    }
}

Strip * _create_strip_common(struct Game * game) {
    Strip* self = malloc(sizeof(Strip));
    self->items = malloc(sizeof(struct Cell) * game->size.x);
    self->render = render_strip;

    fill_strip(self, Null, game);

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

    int width = game->size.x;

    Cell *tmp = (self->direction == UPDATE_RIGHT)
              ? &self->items[width - 1]
              : &self->items[0];
    Cell cpy = *tmp;

    int offset = (tmp != self->items);    
    memmove(
        self->items +  offset,
        self->items + !offset,
        (width - 1) * sizeof(Cell)
    );
    // tmp now becomes the symbol on the edge
    if (self->entity_count == 0) {
        fprintf(stderr, "Strip run out of entities\n");
        endwin();
        exit(0xF00DCAFE);
    }
    int index = (width - !offset) % width;
    // TODO: seperate code below
    if (cpy.uid != 0) {
        if (cpy.uid != tmp->uid) {
            Entity * entity = (Entity*)&cpy;
            entity->width = 2;
            
            if (!add_entity_at(self, entity, self->bg, game, index))
                self->entity_count--;
            else return;
        }
        cpy = (Cell) { .symbol = self->bg };
    }
    self->items[index] = cpy;
}

int entity_fits(
    Strip * self, Entity * entity,
    unsigned index, Symbol bg,
    struct Game * game
) {
    for (unsigned j = 0; j < entity->width; j++) {
        if (self->items[(index + j) % game->size.x].symbol != bg) {
            return 0;
        }
    }
    return 1;
}

int add_entity_at(
    Strip * self,
    Entity * entity,
    Symbol bg,
    struct Game * game,
    int where
) {
    static UID uid = 0; 
    while (1) {
        unsigned index = (where == -1) ? rand() % game->size.x : where;
        
        if (!entity_fits(self, entity, index, bg, game)) {
            if (where == -1)
                continue; // find new position
            return 0; // failed to add an entity on speciefied position
        }
        entity->uid = ++uid;
        for (unsigned j = 0; j < entity->width; j++) {
            Cell * cell = &self->items[(index + j) % game->size.x];
            memcpy(cell, entity, sizeof(Cell));
        }
        return 1;
    }
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

    fill_strip(self, bg, game);

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

void destroy_strip(Strip * self) {
    if (self->items != NULL)
        free(self->items);
    free(self);
}

