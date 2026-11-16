#include <stddef.h>
#include <time.h>

typedef enum {
    BF_CLEAR_COVERED = 0,
    BF_UNCOVERED_FLAG = 1,
    BF_MINE_FLAG = 4,
    BF_FLAGGED_FLAG = 8,

    BF_MINE_COVERED = BF_MINE_FLAG,
    BF_MINE_UNCOVERED = BF_MINE_FLAG | BF_UNCOVERED_FLAG,
    BF_CLEAR_UNCOVERED = BF_UNCOVERED_FLAG,
    BF_MINE_FLAGGED = BF_FLAGGED_FLAG | BF_MINE_COVERED,
    BF_CLEAR_FLAGGED = BF_FLAGGED_FLAG | BF_CLEAR_COVERED,
} BoardField;

typedef enum {
    GS_UNINITIALIZED,
    GS_GAME_IN_PROGRESS,
    GS_WON,
    GS_LOST
} GameState;

typedef struct {
    GameState state;
    size_t w, h;
    size_t mines;
    size_t flags_planted;
    time_t start_time, elapsed_time;
    
    BoardField* mine_locations;
    
} Board;

void board_load_textures(void);

Board* board_new(size_t w, size_t h, size_t mines);

// (fx, fy) - first move
void board_gen(Board* board, size_t fx, size_t fy);

const char* board_game_state_to_string(GameState);
void board_draw_game_info(Board* board);
void board_draw(Board* board);
void board_handle_lmb_press(Board* board);
void board_handle_rmb_press(Board* board);
void board_print(Board* board);