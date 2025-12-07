#include "game/loadingState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constructors

LoadingState::LoadingState() {
   loadFont("andy", "assets/fonts/andy.ttf");
   loadTexture("loading", "assets/sprites/ui/loading.png");
   splash = getRandomLineFromFile("assets/splash.txt");
   wrapText(splash, GetScreenWidth() - 50.f, 40, 1.f);
}

// Update functions

void LoadingState::update() {
   rotation += GetFrameTime() * 360.f;

   // Sometimes brute-forcing is better than over-engineering an automatic way to do everything
   if (load == Load::fonts) {
      text = "Loading Fonts... ";
      loadFonts();
      load = Load::textures;
   } else if (load == Load::textures) {
      text = "Loading Textures... ";
      loadTextures();
      load = Load::sounds;
   } else if (load == Load::sounds) {
      text = "Loading Sounds... ";
      loadSounds();
      load = Load::soundSetup;
   } else if (load == Load::soundSetup) {
      text = "Setting Up Sounds... ";
      saveSound("click", {"click", "click2", "click3"});
      saveSound("hover", {"hover", "hover2"});
      saveSound("jump", {"jump1", "jump2", "jump3"});
      load = Load::music;
   } else if (load == Load::music) {
      text = "Loading Music... ";
      loadMusic();
      playSound("load");
      load = Load::count;
   } else if (load == Load::count) {
      waitTimer += GetFrameTime();
      fadingOut = (waitTimer >= 1.f);
   }
}

// Other functions

void LoadingState::render() {
   Texture2D &tex = getTexture("loading");
   std::string ltext = "Loading Done!";
   if (load != Load::count) {
      ltext = text + std::to_string((int)load) + "/" + std::to_string((int)Load::count);
   }

   drawText(getScreenCenter({0.f, -175.f}), ltext.c_str(), 80);
   drawText(getScreenCenter({0.f, 100.f}), splash.c_str(), 40);
   drawTexture(tex, getScreenCenter(), {70.f, 70.f}, rotation);
}

State* LoadingState::change() {
   return new MenuState();
}
