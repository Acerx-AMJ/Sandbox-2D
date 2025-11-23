#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

#include <vector>
#include "game/state.hpp"
#include "ui/button.hpp"
#include "ui/scrollframe.hpp"

// Menu state

struct MenuState: public State {
   MenuState();
   ~MenuState() = default;

   // Update

   void update() override;
   void updateTitle();
   void updateLevelSelection();

   // Render

   void render() override;
   void renderTitle();
   void renderLevelSelection();

   // Other functions

   State* change() override;
   void loadWorlds();

private:
   enum class Phase { title, levelSelection };

   Button playButton, optionsButton, quitButton;
   Button backButton, newButton;
   std::vector<Button> worldButtons;
   Scrollframe worldFrame;

   Phase phase = Phase::title;
   bool playing = false;
};

#endif
