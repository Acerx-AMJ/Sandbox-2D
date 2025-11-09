#include "game/menuState.hpp"

// Includes

#include <raylib.h>

// Constants

namespace {
   constexpr float fadeTime = .25f;
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::fadingIn:  updateFadingIn();  break;
   case Phase::updating:  updateUpdating();  break;
   case Phase::fadingOut: updateFadingOut(); break;
   }
}

void MenuState::updateFadingIn() {
   fadeTimer += GetFrameTime();
   alpha = 1.f - fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      fadeTimer = alpha = 0.f;
      phase = Phase::updating;
   }
}

void MenuState::updateUpdating() {

}

void MenuState::updateFadingOut() {
   fadeTimer += GetFrameTime();
   alpha = fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      alpha = 1.f;
      quitState = true;
   }
}

// Other functions

void MenuState::render() {
   BeginDrawing();
      ClearBackground(BLACK);
      DrawText("lalaladidada", 50, 50, 200, RED);
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
   EndDrawing();
}

void MenuState::change(States& states) {
   
}
