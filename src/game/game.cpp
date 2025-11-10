#include "game/game.hpp"

// Includes

#include <cstdlib>
#include <raylib.h>
#include "game/loadingState.hpp"
#include "mngr/sound.hpp"
#include "util/render.hpp"

// Constructors

Game::Game() {
   srand(time(nullptr));
   InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), "Terraria Clone");
   ToggleFullscreen();
   SetTargetFPS(60);
   
   InitAudioDevice();
   SetExitKey(KEY_NULL);
   states.push_back(LoadingState::make());
}

Game::~Game() {
   CloseWindow();
   CloseAudioDevice();
}

// Run function

void Game::run() {
   while (not WindowShouldClose()) {
      if (states.front()->quitState) {
         states.front()->change(states);
         states.pop_front();
      }

      if (states.empty()) {
         return;
      }

      SoundManager::get().update();
      states.front()->updateStateLogic();

      BeginDrawing();
         ClearBackground(BLACK);
         states.front()->render();
         drawRect(Fade(BLACK, states.front()->alpha));
      EndDrawing();
   }
}
