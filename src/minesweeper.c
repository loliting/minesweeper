#include <raylib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "board.h"

static void show_help(const char* program_name) {
    printf("usage: %s -d [difficulty]\n", program_name);
    printf("\t-h, --help\t\tShows this message\n");
    printf("\t-d, --difficulty [1,2]\tChoose difficulty\n");
    printf("\t\t\t\t  1 - Normal\n\t\t\t\t  2 - Hard\n");
}

int main(int argc, char* argv[]) {
    if(argc > 3 || argc == 2) {
        show_help(*argv);
        exit(0);
    }
    int difficulty = -1;
    if(argc == 3 && (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--difficulty"))) {
        if(sscanf(argv[2], "%d", &difficulty) != 1 || difficulty < 1 || difficulty > GD_HARD) {
            show_help(*argv);
            exit(0);
        }
    }
    else {
        difficulty = GD_NORMAL;
    }


    SetTargetFPS(60);
    InitWindow(500, 620, "Minesweeper");
    board_load_textures();
    Board *board = board_new(difficulty);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ 30, 30, 30});
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