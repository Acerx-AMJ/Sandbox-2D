#include "game/loadingState.hpp"
#include "SRU/util.hpp"
#include "game/menuState.hpp"
#include "mngr/data.hpp"
#include "ui/popup.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/file.hpp"
#include "SRU/text.hpp"
#include "SRU/render.hpp"

constexpr float splashFontSize = 40.0f;
constexpr float loadingTextFontSize = 80.0f;

// Constructors

LoadingState::LoadingState() {
   loadFont("andy", "assets/fonts/andy.ttf");
   loadTexture("loading", "assets/sprites/ui/loading.png");
   splashText = getRandomLineFromFile("assets/config/splash.txt");
   wrapInPlace(splashText, getFont("andy"), mapRatioToX(0.9), getFontSizeScaled(splashFontSize));
}

void LoadingState::update() {
   iconRotation += dt * 360.0f;

   // Sometimes brute-forcing is better than over-engineering an automatic way to do everything
   if (loadPhase == Load::fonts) {
      loadFonts("assets/fonts/");

      loadingText = "Loading Textures... ";
      loadPhase = Load::textures;
   } else if (loadPhase == Load::textures) {
      loadTextures("assets/sprites/");
      initPopups();
      initParticles();

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

void LoadingState::render() {
   std::string finalLoadingText = loadingText;
   if (loadPhase != Load::count) {
      finalLoadingText = TextFormat("%s%d/%d", loadingText.c_str(), (int)loadPhase, (int)Load::count);
   }
   drawTextResponsive("andy", V2(0.5, 0.33f), finalLoadingText.c_str(), loadingTextFontSize);
   drawTextResponsive("andy", V2(0.5f, 0.6f), splashText.c_str(), splashFontSize);
   drawTextureResponsive("loading", V2(0.5f, 0.5f), V2(0.065), CENTER, WHITE, FULL_SOURCE, WINDOW_AREA, CUBIC_RATIO, iconRotation);
}

State* LoadingState::change() {
   return new MenuState();
}
