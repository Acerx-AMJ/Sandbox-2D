#include "game/loadingState.hpp"
#include "mngr/input.hpp"
#include "ui/popup.hpp"
#include "util/render.hpp"
#include <raylib.h>
#include <cstdlib>
#include <ctime>

int main() {
   srand(time(nullptr));
   SetConfigFlags(FLAG_VSYNC_HINT);
   InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), "Sandbox-2D");
   ToggleFullscreen();
   SetTargetFPS(60);
   InitAudioDevice();
   SetExitKey(KEY_NULL);

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

      updateInput();
      updatePopups(current->realDt);

      if (!anyPopups()) {
         current->updateStateLogic();
      }

      BeginDrawing();
         ClearBackground(BLACK);
         current->render();

         renderPopups();
         drawRect(Fade(BLACK, current->alpha));
      EndDrawing();
   }
   CloseWindow();
   CloseAudioDevice();
}
