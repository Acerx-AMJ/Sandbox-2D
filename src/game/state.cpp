#include "game/state.hpp"
#include "ui/uiconstants.hpp"
#include <raylib.h>

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
      fadeTimer = 0.f;
      alpha = 0.f;
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
