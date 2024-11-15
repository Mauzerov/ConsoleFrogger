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

typedef enum {
    Safe = 0,
    Solid, // Blocking
    Evil,
} CellType;

struct Game;

struct Cell {
    Symbol symbol;
    CellType type;
};

typedef struct Cell Cell;

void render_symbol(Symbol, struct Game *);


#endif