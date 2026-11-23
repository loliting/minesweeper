#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

    InitWindow(500, 620, "Minesweeper");
    board_load_textures();
    Board *board = board_new(difficulty);
    while (!WindowShouldClose()) {
        board_game_loop(board);
    }
    CloseWindow();

    return 0;
}