#include "game/loadingState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/popup.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constructors

LoadingState::LoadingState() {
   loadFont("andy", "assets/fonts/andy.ttf");
   loadTexture("loading", "assets/sprites/ui/loading.png");
   splashText = getRandomLineFromFile("assets/splash.txt");
   wrapText(splashText, GetScreenWidth() - 50.0f, 40, 1.f);
}

void LoadingState::update() {
   iconRotation += dt * 360.0f;

   // Sometimes brute-forcing is better than over-engineering an automatic way to do everything
   if (loadPhase == Load::fonts) {
      loadFonts();

      loadingText = "Loading Textures... ";
      loadPhase = Load::textures;
   } else if (loadPhase == Load::textures) {
      loadTextures();
      initPopups();

      loadingText = "Loading Shaders... ";
      loadPhase = Load::shaders;
   } else if (loadPhase == Load::shaders) {
      loadShaders();

      loadingText = "Loading Sounds... ";
      loadPhase = Load::sounds;
   } else if (loadPhase == Load::sounds) {
      loadSounds();
      loadSavedSounds();

      loadingText = "Loading Music... ";
      loadPhase = Load::music;
   } else if (loadPhase == Load::music) {
      loadMusic();
      playSound("load");

      loadingText = "Loading Done!";
      loadPhase = Load::count;
   } else if (loadPhase == Load::count) {
      finalWaitTimer += realDt;
      fadingOut = (finalWaitTimer >= 1.f);
   }
}

void LoadingState::fixedUpdate() {
   // Loading state does not require any physics
}

void LoadingState::render() const {
   std::string finalLoadingText = loadingText;
   if (loadPhase != Load::count) {
      finalLoadingText = format("{}{}/{}", loadingText, (int)loadPhase, (int)Load::count);
   }

   drawText(getScreenCenter({0.0f, -175.0f}), finalLoadingText.c_str(), 80);
   drawText(getScreenCenter({0.0f, 100.0f}), splashText.c_str(), 40);
   drawTexture(getTexture("loading"), getScreenCenter(), {70.0f, 70.0f}, iconRotation);
}

State* LoadingState::change() {
   return new MenuState();
}
