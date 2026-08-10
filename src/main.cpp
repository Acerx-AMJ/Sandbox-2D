#include "game/loadingState.hpp"
#include <raylib.h>

constexpr int minWindowWidth  = 800;
constexpr int minWindowHeight = 600;

int main() {
   SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
   InitWindow(minWindowWidth, minWindowHeight, "Sandbox-2D");
   SetWindowMinSize(minWindowWidth, minWindowHeight);
   InitAudioDevice();
   SetExitKey(KEY_NULL);
   SetTraceLogLevel(LOG_ERROR);

   State *current = new LoadingState();
   
   while (!WindowShouldClose()) {
      if (current->quitState) {
         State *newState = current->change();
         delete current;
         current = newState;
      }

      if (!current) {
         break;
      }
      current->updateStateLogic();
      current->renderStateLogic();
   }
   CloseWindow();
   CloseAudioDevice();
}
