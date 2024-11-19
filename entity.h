#ifndef ENTITY_H
#define ENTITY_H

// NOTE: Entity should overload most (if not all) of Cell
struct Entity {
    Symbol symbol;
    // TODO: remove uid (if needed use pointer adresses, should be unique enough)
    CellType type;
    // Entity width (number of positions taken up, to the right of .position)
    unsigned width;
    // just a x-axis position (y is obtained from strip)
    // no need for duplication of data
    unsigned position;
    // NOTE: Entity is a linked list:
    // Why? - easiest implementation for removing and adding new entities
    //      + entities do not interact with eachother
    // Why not make a different struct where Entity is the value?
    // getting current entity would be annoying
    // Additional Benefits:
    // + no need to keep the count of entities
    // 
    struct Entity * next;
};

void undo_move(struct Game *);

void lose_game(struct Game *);

void end_game(struct Game *, int);

void handle_entity_collision(struct Entity *, struct Game*);


#define max(a, b) (((a) < (b)) ? (b) : (a))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define clamp(x_min, x, x_max) ( \
    ((x) < (x_min)) ?            \
        (x_min)     :            \
        (((x) > (x_max)) ?       \
            (x_max)      : (x))  \
)

#endif