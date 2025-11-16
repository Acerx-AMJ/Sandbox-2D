#include <fstream>
#include <nlohmann/json.hpp>
#include "game/loadingState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/text.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

// Constructors

LoadingState::LoadingState() {
   ResourceManager::get().loadFont("andy", "assets/fonts/andy.ttf");
   ResourceManager::get().loadTexture("loading", "assets/sprites/loading.png");
   splash = getSplashMessage();
   wrapText(splash, GetScreenWidth() - 50.f, 40, 1.f);
}

// Update functions

void LoadingState::update() {
   rotation += GetFrameTime() * 360.f;

   if (load == Load::fonts) {
      text = "Loading Fonts... ";
      ResourceManager::get().loadFonts();
      load = Load::textures;
   } else if (load == Load::textures) {
      text = "Loading Textures... ";
      ResourceManager::get().loadTextures();
      load = Load::sounds;
   } else if (load == Load::sounds) {
      text = "Loading Sounds... ";
      SoundManager::get().loadSounds();
      load = Load::soundSetup;
   } else if (load == Load::soundSetup) {
      text = "Setting Up Sounds... ";
      SoundManager::get().saveSound("click", {"click", "click2", "click3"});
      SoundManager::get().saveSound("hover", {"hover", "hover2"});
      load = Load::music;
   } else if (load == Load::music) {
      text = "Loading Music... ";
      SoundManager::get().loadMusic();
      SoundManager::get().play("load");
      load = Load::count;
   } else if (load == Load::count) {
      waitTimer += GetFrameTime();
      fadingOut = (waitTimer >= 1.f);
   }
}

// Other functions

void LoadingState::render() {
   auto& tex = ResourceManager::get().getTexture("loading");
   std::string ltext = "Loading Done!";
   if (load != Load::count) {
      ltext = text + std::to_string((int)load) + "/" + std::to_string((int)Load::count);
   }

   drawText(getScreenCenter(0.f, -175.f), ltext.c_str(), 80);
   drawText(getScreenCenter(0.f, 100.f), splash.c_str(), 40);
   drawTexture(tex, getScreenCenter(), {70.f, 70.f}, rotation);
}

void LoadingState::change(States& states) {
   states.push_back(MenuState::make());
}

// Return the error message as the splash as the average user might not
// have the terminal opened
std::string LoadingState::getSplashMessage() {
   std::ifstream file ("assets/splash.json");
   if (not file.is_open()) {
      return "File 'assets/splash.json' does not exist.";
   }

   nlohmann::json data;
   file >> data;
   file.close();

   if (not data.contains("splash") or not data["splash"].is_array()) {
      return "Expected 'assets/splash.json' to contain array 'splash'.";
   }
   
   auto splash = data["splash"][random(0, data["splash"].size() - 1)];
   if (not splash.is_string()) {
      return format("Expected 'assets/splash.json' element '{}' to be of type 'string', but it is '{}' instead.", splash.dump(), splash.type_name());
   }
   return splash;
}
