#include "board.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <raylib.h>

#include "res.h"

#define GAME_STATE_TEXT "GAME STATE: "
#define GAME_TIME_TEXT "ELAPSED TIME: "
#define GAME_DIFFICULTY_TEXT "GAME DIFFICULTY: "

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

static Image flag_image;
static Image tile_hidden_image;
static Image tile_revealed_image;
static Image mine_image;

static Texture2D flag_texture;
static Texture2D tile_hidden_texture;
static Texture2D tile_revealed_texture;
static Texture2D mine_texture;

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

Board* board_new(GameDifficulty diff) {
    Board* board = calloc(1, sizeof(Board));
    board->state = GS_UNINITIALIZED;

    switch(diff) {
    default:
    case GD_NORMAL:
        board->mines = 10;
        board->w = 9;
        board->h = 9;
        break;
    case GD_HARD:
        board->mines = 40;
        board->w = 16;
        board->h = 16;
        break;
    }
    board->difficulty = diff;
    board->flags_planted = 0;
    return board;
}

void board_gen(Board* board, size_t fx, size_t fy) {
    board->start_time = time(NULL);
    board->state = GS_GAME_IN_PROGRESS;
    board->mine_locations = calloc(board->w * board->h, sizeof(BoardField));
    board->mine_locations[fx * board->h + fy] = BF_CLEAR_UNCOVERED;
    board->flags_planted = 0;

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
    char game_difficulty_text[128] = GAME_DIFFICULTY_TEXT;

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

    DrawText(game_state_text, 10, 10, 24, RAYWHITE);
    DrawText(game_time_text, 10, 36, 24, RAYWHITE);
    switch (board->difficulty) {
    default:
    case GD_NORMAL:
        strcpy(game_difficulty_text + sizeof(GAME_DIFFICULTY_TEXT) - 1, "NORMAL");
        break;
    case GD_HARD:
        strcpy(game_difficulty_text + sizeof(GAME_DIFFICULTY_TEXT) - 1, "HARD");
        break;
    }
    DrawText(game_difficulty_text, 10, 62, 24, RAYWHITE);
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
    DrawRectangle(loff, toff, w, w, GRAY);
    
    double scale = fw/32.0f;
    for (size_t i = 0; i < board->w; ++i) {
        for (size_t j = 0; j < board->h; ++j) {
            BoardField cell_state = BF_CLEAR_COVERED;
            if(board->state != GS_UNINITIALIZED)
                cell_state = board->mine_locations[i * board->h + j];
            Vector2 pos = {
                .x = loff + i * fw,
                .y = toff + j * fw
            };

            if(cell_state & BF_UNCOVERED_FLAG) {
                DrawTextureEx(tile_revealed_texture, pos, 0.0f, scale, WHITE);
            }
            else {
                DrawTextureEx(tile_hidden_texture, pos, 0.0f, scale, WHITE);
            }

            if(cell_state & BF_FLAGGED_FLAG) {
                DrawTextureEx(flag_texture, pos, 0.0f, scale, WHITE);
            }

            if(cell_state == BF_MINE_UNCOVERED) {
                DrawTextureEx(mine_texture, pos, 0.0f, scale, WHITE);
            }

            if(cell_state == BF_CLEAR_UNCOVERED) {
                int near_mines = board_count_near_mines(board, i, j);
                // guaranteed to be =< 9
                if(near_mines) {
                    char text[2] = { '0' + near_mines, '\0' };
                    DrawText(text, pos.x + 6, pos.y + 3, 48.0f * fw / 52.0f, BLACK);
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

void board_load_textures(void) {
    flag_image = LoadImageFromMemory(".png", 
        res_flag_png, res_flag_png_len);
    tile_hidden_image = LoadImageFromMemory(".png", 
        res_tile_hidden_png, res_tile_hidden_png_len);
    tile_revealed_image = LoadImageFromMemory(".png",
        res_tile_revealed_png, res_tile_revealed_png_len);
    mine_image = LoadImageFromMemory(".png",
        res_mine_png, res_mine_png_len);
    
    flag_texture = LoadTextureFromImage(flag_image);
    tile_hidden_texture = LoadTextureFromImage(tile_hidden_image);
    tile_revealed_texture = LoadTextureFromImage(tile_revealed_image);
    mine_texture = LoadTextureFromImage(mine_image);
    printf("IS TEXTURE VALID (flag_texture): %d\n", IsTextureValid(flag_texture));
    printf("IS TEXTURE VALID (tile_hidden_texture): %d\n", IsTextureValid(tile_hidden_texture));
    printf("IS TEXTURE VALID (tile_revealed_texture): %d\n", IsTextureValid(tile_revealed_texture));
    printf("IS TEXTURE VALID (mine_texture): %d\n", IsTextureValid(mine_texture));
}

__attribute__((destructor)) static void board_destory_textures(void) {
    UnloadTexture(flag_texture);
    UnloadTexture(tile_hidden_texture);
    UnloadTexture(tile_revealed_texture);

    UnloadImage(flag_image);
    UnloadImage(tile_hidden_image);
    UnloadImage(tile_revealed_image);
}