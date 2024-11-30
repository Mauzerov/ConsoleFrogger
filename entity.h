#ifndef ENTITY_H
#define ENTITY_H

// NOTE: Entity should overload most (if not all) of Cell
struct Entity {
    Symbol symbol;
    CellType type;
    // Entity width (number of positions taken up, to the right of .position)
    unsigned width;
    // just a x-axis position (y is obtained from strip)
    // no need for duplication of data (unless entity can move vertically)
    struct Point pos;
    // action to perform when player is near
    int stop_when_player_near;
    
    int state, velocity;
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

int can_move(struct Entity *);

void undo_move(struct Game *);

void end_game(struct Game *, int);

void handle_entity_collision(struct Entity *, struct Game*);

void player_near_car(struct Entity *);

#endif