#include "game/loadingState.hpp"
#include "mngr/input.hpp"
#include "ui/popup.hpp"
#include "util/render.hpp"
#include <raylib.h>
#include <cstdlib>
#include <ctime>

constexpr int minWindowWidth  = 800;
constexpr int minWindowHeight = 600;

int main() {
   srand(time(nullptr));
   SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
   InitWindow(minWindowWidth, minWindowHeight, "Sandbox-2D");
   SetWindowMinSize(minWindowWidth, minWindowHeight);

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
      current->updateStateLogic();

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
