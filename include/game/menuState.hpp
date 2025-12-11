#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

#include "game/state.hpp"
#include "ui/button.hpp"
#include "ui/checkbox.hpp"
#include "ui/input.hpp"
#include "ui/scrollframe.hpp"
#include <vector>

// Menu state

struct MenuState: public State {
   MenuState();
   ~MenuState() = default;

   // Update

   void update() override;
   void updateTitle();
   void updateLevelSelection();
   void updateLevelCreation();
   void updateGeneratingLevel();

   // Render

   void render() override;
   void renderTitle();
   void renderLevelSelection();
   void renderLevelCreation();
   void renderGeneratingLevel();

   // Other functions

   State* change() override;
   void loadWorlds();
   std::string getRandomWorldName();

private:
   enum class Phase { title, levelSelection, levelCreation, generatingLevel };

   Button playButton, optionsButton, quitButton;
   Button backButton, newButton, createButton;
   std::vector<Button> worldButtons;
   Scrollframe worldFrame;
   Input worldName;
   CheckBox shouldWorldBeFlat;

   Texture &backgroundTexture, &foregroundTexture;
   float scrollingBg = 0.f, scrollingFg = 0.f, currentTime = 0.f;
   int moonPhase = 0;
   bool isNight = false;

   std::string selectedWorld;
   Phase phase = Phase::title;
   bool playing = false;
};

#endif
