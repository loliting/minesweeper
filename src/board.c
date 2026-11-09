#include "board.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <raylib.h>

#define GAME_STATE_TEXT "GAME STATE: "
#define GAME_TIME_TEXT "ELAPSED TIME: "

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

static void board_try_uncover(Board* board, size_t x, size_t y);
static void board_make_move(Board* board, size_t x, size_t y);

const char* board_game_state_to_string(GameState gs) {
    const char* gs_str = NULL;
    switch(gs) {
    case GS_GAME_IN_PROGRESS:
        gs_str = "Game in progress";
        break;
    case GS_LOST:
        gs_str = "Game lost!";
        break;
    case GS_WON:
        gs_str = "Game won!";
        break;
    case GS_UNINITIALIZED:
        gs_str = "Game not started";
        break;
    default:
        gs_str = "Game not started";
        break;
    }
    return gs_str;
}

Board* board_new(size_t w, size_t h, size_t mines) {
    assert(w == h);

    Board* board = calloc(1, sizeof(Board));
    board->w = w;
    board->h = h;
    board->mines = mines;
    board->state = GS_UNINITIALIZED;

    return board;
}

void board_gen(Board* board, size_t fx, size_t fy) {
    board->start_time = time(NULL);
    board->state = GS_GAME_IN_PROGRESS;
    board->mine_locations = calloc(board->w * board->h, sizeof(BoardField));
    board->mine_locations[fx * board->h + fy] = BF_CLEAR_UNCOVERED;

    srand(time(NULL));
    for(size_t i = 0; i < board->mines; ++i) {
        double pos;
        regen:
        pos = rand();
        pos /= RAND_MAX;
        pos *= board->w * board->h;
        pos = round(pos);
        if(board->mine_locations[(int)pos]) {
            goto regen;
        }

        board->mine_locations[(int)pos] = BF_MINE_COVERED;
    }

    board->mine_locations[fx * board->h + fy] = BF_CLEAR_COVERED;
    
    board_try_uncover(board, fx, fy);
    board_try_uncover(board, fx, fy + 1);
    board_try_uncover(board, fx, fy - 1);
    board_try_uncover(board, fx + 1, fy);
    board_try_uncover(board, fx + 1, fy - 1);
    board_try_uncover(board, fx + 1, fy + 1);
    board_try_uncover(board, fx - 1, fy);
    board_try_uncover(board, fx - 1, fy + 1);
    board_try_uncover(board, fx - 1, fy - 1);
    board_try_uncover(board, fx, fy - 1);
}

void board_draw_game_info(Board* board) {
    char game_state_text[128] = GAME_STATE_TEXT;
    char game_time_text[128] = GAME_TIME_TEXT;

    strcpy(game_state_text + sizeof(GAME_STATE_TEXT) - 1,
        board_game_state_to_string(board->state));

    if(board->start_time == 0) {
        strcpy(game_time_text + sizeof(GAME_TIME_TEXT) - 1,
            "00:00:00");
    }
    else {
        char elapsed_time_str[9] = { 0 };
        time_t now = time(NULL);
        unsigned char s, m, h;
        
        if(board->state == GS_GAME_IN_PROGRESS)
            board->elapsed_time = now - board->start_time;
        s = board->elapsed_time % 60;
        m = (board->elapsed_time / 60) % 60;
        h = ((board->elapsed_time / 60) / 60) % 60;
        sprintf(elapsed_time_str, "%02hhu:%02hhu:%02hhu", h, m, s);
        strncpy(game_time_text + sizeof(GAME_TIME_TEXT) - 1, elapsed_time_str, 9);
    }

    DrawText(game_state_text, 10, 10, 24, WHITE);
    DrawText(game_time_text, 10, 36, 24, WHITE);
}

static int board_count_near_mines(Board* board, size_t x, size_t y) {
    int near_mines = 0;
    if(x + 1 < board->w) {
        near_mines += !!(board->mine_locations[(x + 1) * board->w + y] & BF_MINE_FLAG);
        if(y + 1 < board->w)
            near_mines += !!(board->mine_locations[(x + 1) * board->w + y + 1] & BF_MINE_FLAG);
        if(y > 0)
            near_mines += !!(board->mine_locations[(x + 1) * board->w + y - 1] & BF_MINE_FLAG);
    }
    if(x > 0) {
        near_mines += !!(board->mine_locations[(x - 1) * board->w + y] & BF_MINE_FLAG);
        if(y + 1 < board->w)
            near_mines += !!(board->mine_locations[(x - 1) * board->w + y + 1] & BF_MINE_FLAG);
        if(y > 0)
            near_mines += !!(board->mine_locations[(x - 1) * board->w + y - 1] & BF_MINE_FLAG);
    }
    if(y > 0)
        near_mines += !!(board->mine_locations[x * board->w + y - 1] & BF_MINE_FLAG);
    if(y + 1< board->w)
        near_mines += !!(board->mine_locations[x * board->w + y + 1] & BF_MINE_FLAG);
    return near_mines;
}

void board_draw(Board* board) {
    size_t loff = 15, toff = 100;
    size_t w = MIN(GetScreenWidth() - 2*loff, GetScreenHeight() - toff);
    size_t fw = w / board->w;
    DrawRectangle(loff, toff, w, w, LIGHTGRAY);

    for (size_t i = 0; i < board->w; ++i) {
        for (size_t j = 0; j < board->h; ++j) {
            Color c = DARKGRAY;
            BoardField cell_state = BF_CLEAR_COVERED;
            size_t x = loff + i * fw;
            size_t y = toff + j * fw;
            if(board->state != GS_UNINITIALIZED)
                cell_state = board->mine_locations[i * board->h + j];
            switch(cell_state) {
            case BF_MINE_COVERED:
                c = DARKGRAY;
                break;
            case BF_CLEAR_COVERED:
                c = DARKGRAY;
                break;
            case BF_CLEAR_UNCOVERED:
                c = LIGHTGRAY;
                break;
                case BF_MINE_UNCOVERED:
                c = RED;
                break;
                case BF_MINE_FLAGGED:
                case BF_CLEAR_FLAGGED:
                c = GREEN;
                break;
            }
            DrawRectangle(x, y, fw, fw, c);
            DrawRectangleLines(x, y, fw, fw, BLACK);
            if(cell_state == BF_CLEAR_UNCOVERED) {
                int near_mines = board_count_near_mines(board, i, j);
                // guaranteed to be =< 9
                if(near_mines) {
                    char text[2] = { '0' + near_mines, '\0' };
                    DrawText(text, x + 12, y + 6, 48, BLACK);
                }
            }
        }
    }
}

static void board_try_uncover(Board* board, size_t x, size_t y) {
    if(x >= board->w || y >= board->w)
        return;
    if(board->mine_locations[x * board->w + y] != BF_CLEAR_COVERED)
        return;
    int near_mines = board_count_near_mines(board, x, y);
    board->mine_locations[x * board->w + y] = BF_CLEAR_UNCOVERED;
    if(near_mines) {
        return;
    }
    board_try_uncover(board, x, y + 1);
    board_try_uncover(board, x, y - 1);
    board_try_uncover(board, x + 1, y);
    board_try_uncover(board, x + 1, y - 1);
    board_try_uncover(board, x + 1, y + 1);
    board_try_uncover(board, x - 1, y);
    board_try_uncover(board, x - 1, y + 1);
    board_try_uncover(board, x - 1, y - 1);
    board_try_uncover(board, x, y - 1);
    board_try_uncover(board, x, y + 1);
}

static void board_make_move(Board* board, size_t x, size_t y) {
    switch(board->mine_locations[x * board->w + y]) {
    case BF_MINE_COVERED:
        board->mine_locations[x * board->w + y] = BF_MINE_UNCOVERED;
        board->state = GS_LOST;
        for(size_t i = 0; i < board->w * board->w; ++i) {
            if(board->mine_locations[i] == BF_MINE_COVERED)
                board->mine_locations[i] |= BF_UNCOVERED_FLAG;
        }
        break;
    case BF_CLEAR_COVERED:
        board_try_uncover(board, x, y);
        break;
    default:
        break;
    }
}

void board_handle_rmb_press(Board* board) {
    if(board->state != GS_GAME_IN_PROGRESS) {
        return;
    }

    Vector2 mpos = GetMousePosition();
    size_t loff = 15, toff = 100;
    size_t w = MIN(GetScreenWidth() - 2*loff, GetScreenHeight() - toff);
    size_t fw = w / board->w;
    w -= w % fw;

    if(mpos.x >= loff && w + loff >= mpos.x
        && mpos.y >= toff && w + toff >= mpos.y)
    {
        size_t cell_x = (mpos.x - loff) / fw, cell_y = (mpos.y - toff) / fw;
        printf("%zu %zu\n", cell_x, cell_y);
        switch(board->mine_locations[cell_x * board->w + cell_y]) {
        case BF_MINE_FLAGGED:
        case BF_CLEAR_FLAGGED:
            board->mine_locations[cell_x * board->w + cell_y] &= ~BF_FLAGGED_FLAG;
            board->flags_planted -= 1;
            break;
        case BF_CLEAR_COVERED:
        case BF_MINE_COVERED:
            if(board->flags_planted >= board->mines)
                return;
            board->mine_locations[cell_x * board->w + cell_y] |= BF_FLAGGED_FLAG;
            board->flags_planted += 1;
            break;
        default:
            break;
        }
    }
    if(board->flags_planted == board->mines) {
        size_t correct_flags = 0;
        for(size_t i = 0; i < board->w; ++i) {
            for(size_t j = 0; j < board->h; ++j) {
                if(board->mine_locations[i * board->w + j] == BF_MINE_FLAGGED)
                    ++correct_flags;
            }
        }
        if(correct_flags == board->mines) {
            board->state = GS_WON;
        }
    }
    board_print(board);
}

void board_handle_lmb_press(Board* board) {
    Vector2 mpos = GetMousePosition();

    size_t loff = 15, toff = 100;
    size_t w = MIN(GetScreenWidth() - 2*loff, GetScreenHeight() - toff);
    size_t fw = w / board->w;
    w -= w % fw;

    if(mpos.x >= loff && w + loff >= mpos.x
        && mpos.y >= toff && w + toff >= mpos.y)
    {
        size_t cell_x = (mpos.x - loff) / fw, cell_y = (mpos.y - toff) / fw;
        printf("%zu %zu\n", cell_x, cell_y);
        switch (board->state) {
        case GS_UNINITIALIZED:
            board_gen(board, cell_x, cell_y);
            break;
        case GS_LOST:
        case GS_WON:
            free(board->mine_locations);
            board->mine_locations = 0;
            board->state = GS_UNINITIALIZED;
            break;
        case GS_GAME_IN_PROGRESS:
            board_make_move(board, cell_x, cell_y);
            break;
        }
    }
    board_print(board);
}

void board_print(Board* board) {
    if(board->state == GS_UNINITIALIZED) {
        return;
    }
    printf("board:\n");
    for(size_t i = 0; i < board->w; ++i) {
        for(size_t j = 0; j < board->h; ++j) {
            switch(board->mine_locations[j * board->h + i]) {
            case BF_MINE_COVERED:
            case BF_MINE_UNCOVERED:
                printf("M ");
                break;
            case BF_CLEAR_UNCOVERED:
                printf("  ");
                break;
            case BF_CLEAR_COVERED:
                printf("C ");
                break;
            case BF_MINE_FLAGGED:
                printf("F ");
                break;
            case BF_CLEAR_FLAGGED:
                printf("f ");
                break;
            default:
                printf("%d ", board->mine_locations[j * board->h + i]);
                break;
            }
        }
        printf("\n");
    }
}
