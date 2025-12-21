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

   bool isWorldFavorite(const std::string &name);
   void sortWorldsByFavorites();

private:
   enum class Phase { title, levelSelection, levelCreation, generatingLevel };

   Button playButton, optionsButton, quitButton;
   Button backButton, renameButton, deleteButton, favoriteButton, playWorldButton, newButton;
   Button backButtonCreation, createButton;

   std::vector<std::string> favoriteWorlds;
   std::vector<Button> worldButtons;
   bool anySelected = false, deleteClicked = false;
   Button *selectedButton = nullptr; 

   Scrollframe worldFrame;
   Input worldName;
   CheckBox shouldWorldBeFlat;

   Texture &backgroundTexture, &foregroundTexture;
   std::string selectedWorld;
   Phase phase = Phase::title;
   bool playing = false;
};

#endif
