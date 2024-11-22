#include <string.h>

#include "engine.h"
#include "game.h"

void render_symbol(WINDOW * window, Symbol symbol, struct Game * game) {
    wattron(window, COLOR_PAIR(symbol));
    assert(symbol > 0);
    for (int i = 0; i < CELL_HEIGHT; i++) {
        for (int j = 0; j < CELL_WIDTH; j++) {
            mvwaddch(
                window,
                game->cursor.y * CELL_HEIGHT + i,
                game->cursor.x * CELL_WIDTH + j,
                game->textures[symbol][i * CELL_WIDTH + j]
            );
        }
    }
}

void render_strip(WINDOW * window, Strip * self, struct Game * game) {
    int cursor_x = game->cursor.x;
    for (int i = 0; i < game->size.x; i++) {
        render_symbol(window, self->bg, game);
        game->cursor.x++;
    }
    struct Entity * head = self->entities;
    while (head != NULL) {
        for (unsigned i = 0; i < head->width; i++) {
            game->cursor.x = cursor_x + (head->position + i) % game->size.x;
            render_symbol(window, head->symbol, game);
        }
        head = head->next;
    }
}

void render_leaderboard(struct Game * game) {
    WINDOW * window = game->info_panel;
    Player * leaderboard = game->leaderboard;

    mvwaddstr(window, 0, (INFO_PANEL_WIDTH - 12) >> 1, " SCOREBOARD ");
    
    char leaderboard_line[INFO_PANEL_WIDTH * 2] = { 0 };
    for (int i = 0; i < game->player_count; i++) {
        sprintf(
            leaderboard_line,
            " %1d: %-20s  : %05lu",
            i + 1, leaderboard[i].name, leaderboard[i].score
        );
        mvwaddstr(window, 1 + i, 2, leaderboard_line);
    }
}

void render_game_state(struct Game * game) {
    WINDOW * window = game->window;
    char buffer[100] = { 0 };
    int lenght = sprintf(buffer, " Score: %05lu ", game->score);
    mvwaddstr(window,
        0, ((game->size.x * CELL_WIDTH - lenght) >> 1),
        buffer
    );
}

void render_border(WINDOW * window, struct Game * game) {
    int height = game->size.y + 1;
    int width  = game->size.x + 1;

    for (int y = 0; y <= height; y++) {
        for (int x = 0; x <= width; x++) {
            if (!!(x % width) ^ !!(y % height)) {
                game->cursor.x = 1 + x;
                game->cursor.y = 1 + y;
                render_symbol(window, Border, game);
            }
        }
    }
}

void render_game(struct Game * game) {
    WINDOW * window = game->window;

    int visibility = game->config.VISIBLE_STRIPS;
    int scroll = clamp(
        0, game->player.y - game->config.VISIBLE_AHEAD, game->size.y - visibility
    );

    for (int i = 0; i < visibility; i++) {
        game->cursor.y = i;
        game->cursor.x = 0;
        invoke(
            game->strips[i + scroll]->render,
            window, game->strips[i + scroll], game
        );
    }

    game->cursor.x = game->player.x;
    game->cursor.y = game->player.y - scroll;
    render_symbol(window, Frog, game);
    attron(COLOR_PAIR(Null));
}