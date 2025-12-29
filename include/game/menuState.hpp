#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

#include "game/state.hpp"
#include "ui/button.hpp"
#include "ui/checkbox.hpp"
#include "ui/input.hpp"
#include "ui/scrollframe.hpp"
#include <mutex>
#include <vector>

// Menu state

struct MenuState: public State {
   enum class Phase {title, levelSelection, levelCreation, levelRenaming, generatingLevel};

   // Constructors

   MenuState();
   ~MenuState() = default;

   // Update

   void update() override;
   void fixedUpdate() override;

   void updateTitle();
   void updateLevelSelection();
   void updateLevelCreation();
   void updateLevelRenaming();
   void updateGeneratingLevel();

   // Render

   void render() const override;
   void renderTitle() const;
   void renderLevelSelection() const;
   void renderLevelCreation() const;
   void renderLevelRenaming() const;
   void renderGeneratingLevel() const;

   // Change states

   State* change() override;

   // World selection functions

   void loadWorldButtons();
   void sortWorldButtonsByFavorites();

   std::string generateRandomWorldName() const;
   bool isWorldFavorite(const std::string &name) const;

   // Helper functions

   bool isKeyRepeating(int key, float &repeatTimer, float &delayTimer);
   size_t getSelectedButtonIndex() const;

   // Members

   const Texture &backgroundTexture, &foregroundTexture;

   Button playButton, optionsButton, quitButton;
   Button backButton, renameButton, deleteButton, favoriteButton, playWorldButton, newButton;
   Button backButtonCreation, createButtonCreation;
   Button backButtonRenaming, renameButtonRenaming;
   Button *selectedButton = nullptr; 

   Scrollframe worldFrame;
   Input worldName, worldSearchBar, renameInput;
   CheckBox shouldWorldBeFlat;

   std::vector<std::string> favoriteWorlds;
   std::vector<Button> worldButtons;
   std::string selectedWorld, generationSplash, generationInfoText;
   Phase phase = Phase::title;

   std::mutex generationInfoTextMutex;
   struct MapGenerator *generator;
   bool generatedWorld = true;

   bool anySelected = false;
   bool deleteClicked = false;
   bool wasFavoriteBeforeRenaming = false;
   bool playing = false;

   float upKeyTimer = 0.0f;
   float downKeyTimer = 0.0f;
   float upKeyDelayTimer = 0.0f;
   float downKeyDelayTimer = 0.0f;
};

#endif
