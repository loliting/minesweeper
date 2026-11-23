#include <stddef.h>
#include <time.h>

typedef struct Vector2 Vector2;

typedef enum {
    BT_CLEAR_COVERED = 0,
    BT_UNCOVERED_FLAG = 1,
    BT_MINE_FLAG = 4,
    BT_FLAGGED_FLAG = 8,

    BT_MINE_COVERED = BT_MINE_FLAG,
    BT_MINE_UNCOVERED = BT_MINE_FLAG | BT_UNCOVERED_FLAG,
    BT_CLEAR_UNCOVERED = BT_UNCOVERED_FLAG,
    BT_MINE_FLAGGED = BT_FLAGGED_FLAG | BT_MINE_COVERED,
    BT_CLEAR_FLAGGED = BT_FLAGGED_FLAG | BT_CLEAR_COVERED,
} BoardTile;

typedef enum {
    GS_UNINITIALIZED,
    GS_GAME_IN_PROGRESS,
    GS_WON,
    GS_LOST
} GameState;

typedef enum {
    GD_NORMAL = 1,
    GD_HARD = 2,
} GameDifficulty;

typedef struct {
    GameState state;
    GameDifficulty difficulty;
    size_t size;
    size_t mines;
    size_t flags_planted;
    time_t start_time, elapsed_time;
    
    BoardTile* tiles;
} Board;

void board_load_textures(void);

Board* board_new(GameDifficulty diff);

// (fx, fy) - first move
void board_gen(Board* board, Vector2 first_move);

const char* board_game_state_to_string(GameState);
void board_draw_game_info(Board* board);
void board_draw(Board* board);
void board_handle_lmb_press(Board* board);
void board_handle_rmb_press(Board* board);
void board_print(Board* board);