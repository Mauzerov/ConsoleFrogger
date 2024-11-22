#ifndef CELL_H
#define CELL_H

#include "engine.h"

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
    short color;
};

typedef struct Cell Cell;

void render_symbol(WINDOW *, Symbol, struct Game *);

#endif