#include "game/loadingState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/popup.hpp"
#include "util/config.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constructors

LoadingState::LoadingState() {
   loadFont("andy", "assets/fonts/andy.ttf");
   loadTexture("loading", "assets/sprites/ui/loading.png");
   splash = getRandomLineFromFile("assets/splash.txt");
   wrapText(splash, GetScreenWidth() - splashWrapOffset, 40, 1.f);
}

// Update functions

void LoadingState::update() {
   rotation += GetFrameTime() * loadingIconRotationSpeed;

   // Sometimes brute-forcing is better than over-engineering an automatic way to do everything
   if (load == Load::fonts) {
      text = "Loading Fonts... ";
      loadFonts();
      load = Load::textures;
   } else if (load == Load::textures) {
      text = "Loading Textures... ";
      loadTextures();
      initPopups();
      load = Load::sounds;
   } else if (load == Load::sounds) {
      text = "Loading Sounds... ";
      loadSounds();
      load = Load::soundSetup;
   } else if (load == Load::soundSetup) {
      text = "Setting Up Sounds... ";

      // All saved sounds go here
      saveSound("click", {"click1", "click2", "click3"});
      saveSound("hover", {"hover1", "hover2"});
      saveSound("trash", {"trash1", "trash2", "trash3"});
      saveSound("jump", {"jump1", "jump2", "jump3", "jump4"});
      saveSound("footstep", {"footstep1", "footstep2", "footstep3", "footstep4"});
      saveSound("pickup", {"pickup1", "pickup2", "pickup3", "pickup4"});

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

void LoadingState::render() const {
   Texture2D &tex = getTexture("loading");
   std::string ltext = "Loading Done!";
   if (load != Load::count) {
      ltext = text + std::to_string((int)load) + "/" + std::to_string((int)Load::count);
   }

   drawText(getScreenCenter(loadingTextOffset), ltext.c_str(), 80);
   drawText(getScreenCenter(splashTextOffset), splash.c_str(), 40);
   drawTexture(tex, getScreenCenter(), loadingIconSize, rotation);
}

State* LoadingState::change() {
   return new MenuState();
}
