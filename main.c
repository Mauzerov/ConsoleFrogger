#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <curses.h>
#endif

#include "main.h"

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

    for (int i = 1; i < GAME_HEIGHT - 1; i++) {
        Row * row = &game->rows[i];

        switch (row->type) {
        case River:
        case Road:{
            Symbol player_symbol = player_row->items[game->x].symbol;
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
                    return;
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

void render_cell(Symbol symbol, int y, int x) {
    const char * text = Textures[symbol];

    if (symbol < 1)
        symbol = 1;

    attron(COLOR_PAIR(symbol));

    for (int j = 0; j < CELL_SIZE; j++) {
        for (int i = 0; i < CELL_SIZE; i++) {
            move(CELL_SIZE * y + i, CELL_SIZE * x + j);
            addch(text[CELL_SIZE * i + j]);
        }
    }
}

void render_game(Game * game) {
    for (int i = 0; i < GAME_HEIGHT; i++) {
        for (int j = 0; j < GAME_WIDTH; j++) {
            Symbol symbol = game->rows[i].items[j].symbol;
            render_cell(symbol, j, i);
        }
        render_cell(0, GAME_WIDTH, i);
    }
    for (int i = 0; i <= GAME_WIDTH; i++) {
        render_cell(0, i, GAME_HEIGHT);
    }

    render_cell(Frog, game->x, game->y);
}

void handle_key_down(Game * game, int keycode) {
    int movex = 0, movey = 0;
    if (keycode == 'a') {
        game->y += movey = -(game->y > 0);
    } else
    if (keycode == 'd') {
        game->y += movey = +(game->y + 1 < GAME_HEIGHT);
    } else
    if (keycode == 's') {
        game->x += movex = +(game->x + 1 < GAME_WIDTH);
    } else
    if (keycode == 'w') {
        game->x += movex = -(game->x > 0);
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
    srand(rand());
    srand(rand());
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
    noecho();
    int key;
    do {
        update_game(&game);
        render_game(&game);
        refresh();
        key = getch();
        handle_key_down(&game, key);
    } while (!game.over);
    endwin();
    
    return 0;
}