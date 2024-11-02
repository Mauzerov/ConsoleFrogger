#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <curses.h>
#endif

#define GAME_WIDTH 10
#define GAME_HEIGHT 10

#define WINDOW_WIDTH GAME_WIDTH
#define WINDOW_HEIGHT (GAME_HEIGHT >> 1)

typedef enum {
    Null  = ' ',
    Frog  = '@',
    Tree  = '^',
    Water = '~',
    Log   = '=',
    Car   = '>',
    Taxi  = 'T',
    Curb  = '_',
} Symbol;

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

void init_row(Row * row) {
    switch (row->type) {
    case Forest: {
        for (int i = 0; i < GAME_WIDTH; i++) {
            if (rand() % 5 == 0)
                row->items[i].symbol = Tree;
        }
    } break;
    case Road: {
        for (int i = 0; i < GAME_WIDTH; i++) {
            row->items[i].symbol = (rand() % 3 == 0) ? 
                ((rand() % 3 == 0) ?
                    Car : Taxi) : Curb;
        }
    } break;
    case River: {
        for (int i = 0; i < GAME_WIDTH; i++) {
            row->items[i].symbol = (rand() % 3 == 0) ? Log : Water;
        }
    } break;
    default: break;
    }
}

void init_game(Game * game) {
    // 0-th has to be plains
    for (int i = 1; i < GAME_HEIGHT - 1; i++) {
        Row * row = &game->rows[i];
        row->type = (RowType)(rand() % RowType_Count);

        init_row(row);
    }
}

void update_game(Game * game) {
    Row * player_row = &game->rows[game->y];
    Symbol player_symbol = player_row->items[game->x].symbol;
    
    for (int i = 1; i < GAME_HEIGHT - 1; i++) {
        Row * row = &game->rows[i];

        switch (row->type) {
        case River:
        case Road:{
            // move player along
            if (game->y == i) {
                switch (player_symbol) {
                case Log:
                case Taxi:
                    game->x = (game->x + 1) % GAME_WIDTH;
                    break;
                case Car:
                case Water:
                    game->over = 1;
                    break;
                default:
                    break;
                }
            }

            Cell last_item = row->items[GAME_WIDTH - 1];
            memmove(row->items + 1, &row->items, sizeof(Cell) * (GAME_WIDTH - 1));
            row->items[0] = last_item;
            break;
        }
        default: break;
        }
    }
}

void render_game(Game * game) {
    for (int i = 0, j; i < GAME_HEIGHT; i++) {
        for (j = 0; j < GAME_WIDTH; j++) {
            move(i, j);
            attron(COLOR_PAIR(game->rows[i].items[j].symbol));
            addch((char)game->rows[i].items[j].symbol);
        }
        attron(COLOR_PAIR(Null));
        addch('#');
    }
    for (int i = 0; i < GAME_WIDTH + 1; i++) {
        move(GAME_HEIGHT, i);
        addch('#');
    }
    move(game->y, game->x);
    attron(COLOR_PAIR(Frog));
    addch((char)Frog);
}

void handle_key_down(Game * game, int keycode) {
    int movex = 0, movey = 0;
    if (keycode == 'a') {
        game->x += movex = -(game->x > 0);
    } else
    if (keycode == 'd') {
        game->x += movex = (game->x + 1 < GAME_WIDTH);
    } else
    if (keycode == 's') {
        game->y += movey = (game->y + 1 < GAME_HEIGHT);;
    } else
    if (keycode == 'w') {
        game->y += movey = -(game->y > 0);
    }
    // Collision detection.
    switch (game->rows[game->y].items[game->x].symbol) {
    case Tree:
        game->x -= movex;
        game->y -= movey;
        break;
    default:
        break;
    }
}

int main() {
    srand(time(NULL));
    Game game = { 0 };
    for (int i = 0; i < GAME_HEIGHT; i++) {
        for (int j = 0; j < GAME_WIDTH; j++) {
            game.rows[i].items[j].symbol = Null;
        }
    }
    init_game(&game);

    initscr();
    start_color();

    init_pair(Null ,  COLOR_WHITE , COLOR_BLACK);
    init_pair(Frog ,  COLOR_GREEN , COLOR_BLACK);
    init_pair(Tree ,  COLOR_GREEN , COLOR_BLACK);
    init_pair(Water,  COLOR_CYAN  , COLOR_BLUE );
    init_pair(Log  ,  COLOR_YELLOW, COLOR_BLUE );
    init_pair(Car  ,  COLOR_RED   , COLOR_BLACK);
    init_pair(Taxi ,  COLOR_GREEN , COLOR_BLACK);
    init_pair(Curb ,  COLOR_WHITE , COLOR_BLACK);

    curs_set(0);
    int key;
    printf("%d\n", noecho());
    do {
        render_game(&game);
        refresh();
        key = getch();
        handle_key_down(&game, key);
        update_game(&game);
    } while (!game.over);
    endwin();
    
    return 0;
}