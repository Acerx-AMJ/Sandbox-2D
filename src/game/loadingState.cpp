#include "game/loadingState.hpp"
#include "game/menuState.hpp"
#include "mngr/data.hpp"
#include "ui/popup.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/file.hpp"
#include "SRU/text.hpp"

// Constructors

LoadingState::LoadingState() {
   loadFont("andy", "assets/fonts/andy.ttf");
   loadTexture("loading", "assets/sprites/ui/loading.png");
   splashText = getRandomLineFromFile("assets/splash.txt");

   float fontSize = getFontSizeScaled(40.0f);
   wrapInPlace(splashText, getFont("andy"), GetScreenWidth() - 50.0f * getWidthRatio(), fontSize, fitSpacing(fontSize));
}

void LoadingState::update() {
   iconRotation += dt * 360.0f;

   // Sometimes brute-forcing is better than over-engineering an automatic way to do everything
   if (loadPhase == Load::fonts) {
      loadFonts("assets/fonts/");

      loadingText = "Loading Textures... ";
      loadPhase = Load::textures;
   } else if (loadPhase == Load::textures) {
      loadTextures("assets/textures/");
      initPopups();

      loadingText = "Loading Shaders... ";
      loadPhase = Load::shaders;
   } else if (loadPhase == Load::shaders) {
      loadShaders("assets/shaders/");

      loadingText = "Loading Sounds... ";
      loadPhase = Load::sounds;
   } else if (loadPhase == Load::sounds) {
      loadSounds("assets/sounds/");

      loadingText = "Loading Music... ";
      loadPhase = Load::music;
   } else if (loadPhase == Load::music) {
      loadingText = "Loading Game Data... ";
      loadPhase = Load::data;
   } else if (loadPhase == Load::data) {
      loadData();
      playSound("success");

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

void LoadingState::updateResponsiveness() {
   // Loading state handles its responsiveness straight in render
}

void LoadingState::render() {
   std::string finalLoadingText = loadingText;
   if (loadPhase != Load::count) {
      finalLoadingText = TextFormat("%s%d/%d", loadingText.c_str(), (int)loadPhase, (int)Load::count);
   }

   drawText(getScreenCenter({0.0f, getHeightRatio() * -175.0f}), finalLoadingText.c_str(), getFontSize(80));
   drawText(getScreenCenter({0.0f, getHeightRatio() * 100.0f}), splashText.c_str(), getFontSize(40));
   drawTexture(getTexture("loading"), getScreenCenter(), applyCubicResponsiveness({70.0f, 70.0f}), iconRotation);
}

State* LoadingState::change() {
   return new MenuState();
}
