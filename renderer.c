#include <string.h>

#include "engine.h"
#include "game.h"

void render_symbol(
    WINDOW * window,
    Symbol symbol,
    Symbol background,
    struct Game * game,
    int y_off
) {
    assert(symbol > 0);
    
    SET_COLOR_PAIR(symbol, symbol, background);
    SET_TEXTCOLOR(window, symbol);
    
    for (int i = 0; i < CELL_HEIGHT; i++) {
        for (int j = 0; j < CELL_WIDTH; j++) {
            mvwaddch(
                window,
                (game->cursor.y - y_off) * CELL_HEIGHT + i,
                game->cursor.x * CELL_WIDTH + j,
                game->textures[symbol][i * CELL_WIDTH + j]
            );
        }
    }
}

void render_stork(WINDOW * window, struct Game * game, int y_off) {
    struct Point * stork = &game->stork->pos;
    Symbol background = game->strips[stork->y]->background_color;

    render_symbol(window, Stork, background, game, y_off);
}

void render_player(WINDOW * window, struct Game * game, int y_off) {
    render_symbol(window, Frog, game->strips[game->player.y]->background_color, game, y_off);
}

void render_strip(WINDOW * window, Strip * self, struct Game * game, int y_off) {
    int cursor_x = game->cursor.x;
    // Render Strip Background
    for (int i = 0; i < game->size.x; i++) {
        render_symbol(window, self->background, self->background_color, game, y_off);
        game->cursor.x++;
    }
    // Render Entities
    struct Entity * head = self->entities;
    while (head != NULL) {
        for (unsigned i = 0; i < head->width; i++) {
            game->cursor.x = cursor_x + (head->pos.x + i) % game->size.x;
            render_symbol(window, head->symbol, self->background_color, game, y_off);
        }
        head = head->next;
    }
}

void render_leaderboard(struct Game * game) {
    WINDOW * window = game->info_panel;
    Player * leaderboard = game->leaderboard;

    mvwaddstr(window, 0, (INFO_PANEL_WIDTH - 12) >> 1, " SCOREBOARD ");
    
    char leaderboard_line[INFO_PANEL_WIDTH * 2] = { 0 };
    for (int i = 0; i < game->leaderboard_count; i++) {
        sprintf(
            leaderboard_line,
            " %1d: %-20s  : %05lu",
            i + 1, leaderboard[i].name, leaderboard[i].score
        );
        mvwaddstr(window, 1 + i, 2, leaderboard_line);
    }
}

void render_game_state(struct Game * game) {
    WINDOW * window = wgetparent(game->window);
    
    char buffer[MAX_GAME_WIDTH * CELL_WIDTH] = { 0 };

    int length = sprintf(
        buffer,
        " Level %d | Score: %05lu | %s ",
        game->config.LEVEL_COUNT - game->level + 1,
        game->score,
        game->travel_willingness ? "Waiting" : " Moving"
    );
    // Display the formatted game state information at the top center of the window
    mvwaddstr(window, 0, ((game->size.x * CELL_WIDTH - length + 2) >> 1), buffer);
        length = sprintf(buffer, " Author: Mateusz Mazurek | 203223 ");
        // Display author information at the bottom center of the window
    mvwaddstr(window,
        game->config.VISIBLE_STRIPS * CELL_HEIGHT + 1,
        ((game->size.x * CELL_WIDTH - length + 2) >> 1),
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
                render_symbol(window, Border, Null, game, 0);
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
    // Render each visible strip and the stork if it is within the visible area
    // to allow for a bigger playable area, scrolling is implemented
    for (int i = 0; i < visibility; i++) {
        game->cursor.y = i;
        game->cursor.x = 0;
        render_strip(
            window, game->strips[i + scroll], game, 0
        );
        if (game->stork->pos.y == i + scroll){
            memcpy(&game->cursor, &game->stork->pos, sizeof(struct Point));
            render_stork(window, game, scroll);
        }
    }

    memcpy(&game->cursor, &game->player, sizeof(struct Point));
    render_player(window, game, scroll);
}

int empty_message_box(WINDOW * w, int posy, int posx) {
    SET_COLOR_PAIR(Frog, Frog, Null);
    SET_TEXTCOLOR(w, Frog);

    mvwaddstr (w, posy++, posx, "                            ");
    mvwaddstr (w, posy++, posx, " +========================+ ");
    mvwaddstr (w, posy++, posx, " |                        | ");
    mvwaddstr (w, posy++, posx, " +========================+ ");
    mvwaddstr (w, posy++, posx, "                            ");
    return posy;
}

void confirm(WINDOW * w, const char * message, int key) {
    int width, height;
    getmaxyx(w, height, width);
    int posy = (height - TEXT_BOX_HEIGHT) / 2;
    int posx = (width - TEXT_BOX_WIDTH) / 2;

    ALLOW_INPUT(w, TRUE);
    posy = empty_message_box(w, posy, posx);
    mvwaddstr (w, height / 2 - 1, posx + 2, message);
    noecho(); while (wgetch(w) != key) ;
    ALLOW_INPUT(w, FALSE);
}
