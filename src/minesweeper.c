#include <raylib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "board.h"

int main(int argc, char* argv[]) {
    SetTargetFPS(60);
    InitWindow(500, 620, "Minesweeper");
    board_load_textures();

    Board *board = board_new(9, 9, 10);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GRAY);

        board_draw_game_info(board);

        board_draw(board);
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            board_handle_lmb_press(board);
        if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
            board_handle_rmb_press(board);

        EndDrawing();
    }
    CloseWindow();

    return 0;
}