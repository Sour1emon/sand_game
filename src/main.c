#include <raylib.h>

int main() {

  InitWindow(800, 600, "Hello, Raylib");

  // Main loop
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello Raylib!", 350, 280, 20, LIGHTGRAY);
    EndDrawing();
  }

  CloseWindow(); // Close window and clean up

  return 0;
}
