#pragma once
#include "game/state.hpp"
#include "raylib.h"
#include <string>

struct LoadingState: public State {
   enum class Load { fonts, textures, shaders, sounds, music, data, count };

   LoadingState();
   ~LoadingState() = default;

   // Functions

   void update() override;
   void render() override;
   State* change() override;

   // Members

   std::string splashText;
   std::string loadingText = "Loading Fonts... ";
   Load loadPhase = Load::fonts;

   float finalWaitTimer = 0.f;
   float iconRotation = 0.f;

   // Assets

   Font font;
   Texture loadingTexture;
};
