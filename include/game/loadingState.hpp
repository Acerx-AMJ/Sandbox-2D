#ifndef GAME_LOADINGSTATE_HPP
#define GAME_LOADINGSTATE_HPP

#include "game/state.hpp"
#include <string>

// Loading state

struct LoadingState: public State {
   LoadingState();
   ~LoadingState() = default;

   // Update functions

   void update() override;

   // Other functions

   void render() const override;
   State* change() override;

private:
   enum class Load { fonts, textures, sounds, soundSetup, music, count };

   std::string splash;
   std::string text = "Loading Fonts... ";
   Load load = Load::fonts;
   float waitTimer = 0.f;
   float rotation = 0.f;
};

#endif
