#ifndef CELL_H
#define CELL_H

typedef unsigned long long UID;

typedef enum {
    Border = 0,
    Null = 1,
    Frog,
    Tree,
    Water,
    Log,
    Curb,
    Car,
    Taxi,
    Symbol_Count
} Symbol;

struct Game;

struct Cell {
    Symbol symbol;
    UID uid;
};

typedef struct Cell Cell;

void render_cell(Cell *, struct Game *);


#endif