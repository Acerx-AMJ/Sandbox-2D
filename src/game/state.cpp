#include "game/state.hpp"
#include <raylib.h>

// Constants

constexpr float maxDT    = 0.25f;
constexpr float fadeTime = 0.4f;

// Update functions

void State::updateStateLogic() {
   realDt = GetFrameTime();

   dt = realDt;
   if (dt > maxDT) {
      dt = maxDT;
   }

   if (fadingIn) {
      updateFadingIn();
      return;
   } else if (fadingOut) {
      updateFadingOut();
      return;
   }

   accumulator += dt;
   while (accumulator >= fixedUpdateDT) {
      fixedUpdate();
      accumulator -= fixedUpdateDT;
   }
   update();
}

void State::updateFadingIn() {
   fadeTimer += realDt;
   alpha = 1.f - fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      fadeTimer = 0.f;
      alpha = 0.f;
      fadingIn = false;
   }
}

void State::updateFadingOut() {
   fadeTimer += realDt;
   alpha = fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      alpha = 1.f;
      fadingOut = false;
      quitState = true;
   }
}
