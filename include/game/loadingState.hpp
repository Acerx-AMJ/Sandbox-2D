#ifndef GAME_LOADINGSTATE_HPP
#define GAME_LOADINGSTATE_HPP

#include "game/state.hpp"
#include <string>

// Loading state

struct LoadingState: public State {
   enum class Load { fonts, textures, shaders, sounds, music, count };

   // Constructors

   LoadingState();
   ~LoadingState() = default;

   // Functions

   void update() override;
   void fixedUpdate() override;

   void render() override;
   State* change() override;

   // Members

   std::string splashText;
   std::string loadingText = "Loading Fonts... ";
   Load loadPhase = Load::fonts;

   float finalWaitTimer = 0.f;
   float iconRotation = 0.f;
};

#endif
