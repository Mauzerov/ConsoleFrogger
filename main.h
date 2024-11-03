#ifndef MAIN_h
#define MAIN_h

#define GAME_WIDTH 10
#define GAME_HEIGHT 30
#define CELL_SIZE 2

#define WINDOW_WIDTH GAME_WIDTH
#define WINDOW_HEIGHT (GAME_HEIGHT >> 1)

typedef enum {
    Null = 1,
    Frog,
    Tree,
    Water,
    Log,
    Car,
    Taxi,
    Curb,
    Symbol_Count
} Symbol;

const char Textures[Symbol_Count][CELL_SIZE * CELL_SIZE + 1] = {
    "##"
    "##",
    "  "
    "  ",
    "@@"
    "@@",
    "%%"
    "^^",
    "~~"
    "~~",
    "##"
    "##",
    "<>"
    "oo",
    "TX"
    "oo",
    "()"
    "()",
};

typedef struct {
    Symbol symbol;
    int color;
} Cell;

typedef enum {
    Plains = 0,
    Forest,
    Road,
    River,
    RowType_Count
} RowType;

typedef struct {
    RowType type;
    Cell items[GAME_WIDTH];
} Row;

typedef struct {
    int x, y;
    Row rows[GAME_HEIGHT];
    int over;
} Game;

#endif