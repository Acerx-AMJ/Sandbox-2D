#ifndef GAME_LOADINGSTATE_HPP
#define GAME_LOADINGSTATE_HPP

#include <string>
#include "game/state.hpp"

// Loading state

struct LoadingState: public State {
   LoadingState();
   ~LoadingState() = default;

   // Update functions

   void update() override;

   // Other functions

   void render() override;
   State* change() override;
   std::string getSplashMessage();

private:
   enum class Load { fonts, textures, sounds, soundSetup, music, count };

   std::string splash;
   std::string text = "Loading Fonts... ";
   Load load = Load::fonts;
   float waitTimer = 0.f;
   float rotation = 0.f;
};

#endif
