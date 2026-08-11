#include "game/state.hpp"
#include "mngr/input.hpp"
#include "ui/popup.hpp"
#include "SRU/particles.hpp"
#include <algorithm>
#include <raylib.h>

// Constants

constexpr float maxDT    = 0.25f;
constexpr float fadeTime = 0.4f;

// Update functions

void State::updateStateLogic() {
   updateInput();
   updatePopups(realDt);

   int width = GetScreenWidth();
   int height = GetScreenHeight();

   if (width != lastWidth || height != lastHeight) {
      lastWidth = width;
      lastHeight = height;
      updateResponsiveness();
   }

   if (anyPopups()) {
      return;
   }

   realDt = GetFrameTime();
   dt = std::min(maxDT, realDt);

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
   updateParticles(dt);
   update();
}

void State::renderStateLogic() {
   BeginDrawing();
      ClearBackground(BLACK);
      render();
      renderPopups();
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
   EndDrawing();
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
