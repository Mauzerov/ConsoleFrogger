#include <wchar.h>

#include "engine.h"

#include "strip.h"
#include "cell.h"
#include "game.h"


void render_cell (Cell  * cell, struct Game * game) {
    static char symbols[]  = "# @&~=_OT";
    attron(COLOR_PAIR(cell->symbol));
    move(game->cursor.y, game->cursor.x);
    addch(symbols[cell->symbol]);
}

void render_strip(Strip * self, struct Game * game) {
    for (int i = 0; i < game->size.x; i++) {
        render_cell(&self->items[i], game);
        game->cursor.x++;
    }
}

Strip * _create_strip_common(struct Game * game) {
    Strip* self = malloc(sizeof(Strip));
    self->items = malloc(sizeof(struct Cell) * game->size.x);
    self->render = render_strip;

    for (int i = 0; i < game->size.x; i++) {
        self->items[i].symbol = Null;
    }
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
    int width = game->size.x;
    assert(self->direction != 0 && "Movable Strip cannot be static!");

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

    self->items[(width - !offset) % width] = cpy;
}

void fill_strip(Strip * self, Symbol with, struct Game * game) {
    for (int i = 0; i < game->size.x; i++) {
        self->items[i].symbol = with;
    }
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

void add_entity(Strip * self, Entity * entity, Symbol bg, struct Game * game) {
    while (1) {
        unsigned index = rand() % game->size.x;
        
        if (!entity_fits(self, entity, index, bg, game))
            continue;

        for (unsigned j = 0; j < entity->width; j++) {
            self->items[(index + j) % game->size.x].symbol = entity->symbol;
        }
        return;
    }
}

Strip * _create_strip_movable(
    Symbol bg, Entity * fg,
    size_t n_fg, size_t fg_count,
    struct Game * game
) {
    Strip * self = _create_strip_common(game);

    self->direction = (rand() & 1) ? UPDATE_RIGHT : UPDATE_LEFT;
    self->update = _update_strip_moveable;

    fill_strip(self, bg, game);

    for (size_t i = 0; i < fg_count; i++) {
        Entity entity = fg[rand() % n_fg];
        add_entity(self, &entity, bg, game);
    }

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
        { Log, 2 },
        { Log, 3 },
    };
    return _create_strip_movable(
        Water,
        fg, sizeof(fg) / sizeof (fg[0]) ,
        LOGS_PER_STRIP,
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
        { Car, 1 },
        { Taxi, 1 }
    };
    return _create_strip_movable(
        Curb,
        fg, sizeof(fg) / sizeof (fg[0]),
        CARS_PER_STRIP,
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
        { Tree, 1 },
    };
    Strip * self = _create_strip_common(game);

    for (int i = 0 ; i < TREES_PER_STRIP; i++) {
        add_entity(self, fg, Null, game);
    }
    return self;
}

void destroy_strip(Strip * self) {
    if (self->items != NULL)
        free(self->items);
    free(self);
}

