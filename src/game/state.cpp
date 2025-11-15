#include "game/state.hpp"

// Includes

#include <raylib.h>

// Constants

constexpr float fadeTime = .4f;

// Update functions

void State::updateStateLogic() {
   if (fadingIn) {
      updateFadingIn();
   } else if (fadingOut) {
      updateFadingOut();
   } else {
      update();
   }
}

void State::updateFadingIn() {
   fadeTimer += GetFrameTime();
   alpha = 1.f - fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      fadeTimer = alpha = 0.f;
      fadingIn = false;
   }
}

void State::updateFadingOut() {
   fadeTimer += GetFrameTime();
   alpha = fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      alpha = 1.f;
      fadingOut = false;
      quitState = true;
   }
}
