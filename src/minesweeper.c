#include <raylib.h>

int main(int argc, char* argv[]) {
    SetTargetFPS(60);
    InitWindow(800, 600, "Minesweeper");

    while (!WindowShouldClose()) {
        BeginDrawing();
        EndDrawing();
    }
    CloseWindow();

    return 0;
}