#include "game/loadingState.hpp"
#include "mngr/sound.hpp"
#include "util/render.hpp"
#include <raylib.h>
#include <cstdlib>

int main() {
   // Initialize the game
   
   srand(time(nullptr));
   SetConfigFlags(FLAG_VSYNC_HINT);
   InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), "Sandbox-2D");
   ToggleFullscreen();
   SetTargetFPS(60);
   
   InitAudioDevice();
   SetExitKey(KEY_NULL);
   State* current = new LoadingState();

   // Run the game

   while (not WindowShouldClose()) {
      if (current->quitState) {
         auto* newState = current->change();
         delete current;
         current = newState;
      }

      if (not current) {
         break;
      }

      updateMusic();
      current->updateStateLogic();

      BeginDrawing();
         ClearBackground(BLACK);
         current->render();
         drawRect(Fade(BLACK, current->alpha));
      EndDrawing();
   }

   // De-initialize the game

   if (current) {
      delete current;
   }
   CloseWindow();
   CloseAudioDevice();
}
