#include "game/loadingState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/popup.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constants

constexpr float splashWrapOffset         = 50.0f;
constexpr float loadingIconRotationSpeed = 360.0f;
constexpr Vector2 loadingIconSize        = {70.0f, 70.0f};
constexpr Vector2 loadingTextOffset      = {0.0f, -175.0f};
constexpr Vector2 splashTextOffset       = {0.0f, 100.0f};

// Constructors

LoadingState::LoadingState() {
   loadFont("andy", "assets/fonts/andy.ttf");
   loadTexture("loading", "assets/sprites/ui/loading.png");
   splashText = getRandomLineFromFile("assets/splash.txt");
   wrapText(splashText, GetScreenWidth() - splashWrapOffset, 40, 1.f);
}

void LoadingState::update() {
   iconRotation += GetFrameTime() * loadingIconRotationSpeed;

   // Sometimes brute-forcing is better than over-engineering an automatic way to do everything
   if (loadPhase == Load::fonts) {
      loadingText = "Loading Fonts... ";
      loadFonts();
      loadPhase = Load::textures;
   } else if (loadPhase == Load::textures) {
      loadingText = "Loading Textures... ";
      loadTextures();
      initPopups();
      loadPhase = Load::sounds;
   } else if (loadPhase == Load::sounds) {
      loadingText = "Loading Sounds... ";
      loadSounds();
      loadSavedSounds();
      loadPhase = Load::music;
   } else if (loadPhase == Load::music) {
      loadingText = "Loading Music... ";
      loadMusic();
      playSound("load");
      loadPhase = Load::count;
   } else if (loadPhase == Load::count) {
      finalWaitTimer += GetFrameTime();
      fadingOut = (finalWaitTimer >= 1.f);
   }
}

void LoadingState::render() const {
   std::string finalLoadingText = "Loading Done!";
   if (loadPhase != Load::count) {
      finalLoadingText = format("{}{}/{}", loadingText, (int)loadPhase, (int)Load::count);
   }

   drawText(getScreenCenter(loadingTextOffset), finalLoadingText.c_str(), 80);
   drawText(getScreenCenter(splashTextOffset), splashText.c_str(), 40);
   drawTexture(getTexture("loading"), getScreenCenter(), loadingIconSize, iconRotation);
}

State* LoadingState::change() {
   return new MenuState();
}
