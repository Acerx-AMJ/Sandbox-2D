#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "ui/popup.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/input.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <filesystem>

// Constructors

MenuState::MenuState()
: backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()) {
   const Vector2 center = getScreenCenter();

   // Init title screen
   playButton.rectangle = {center.x, center.y, buttonWidth, buttonHeight};
   playButton.text = "Play";
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   optionsButton.text = "Options";
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   quitButton.text = "Quit";

   playButton.texture = optionsButton.texture = quitButton.texture = &getTexture("button");

   // Init world selection screen
   worldFrame.rectangle = {worldFramePosition.x, worldFramePosition.y, GetScreenWidth() - worldFrameSizeOffset.x, GetScreenHeight() - worldFrameSizeOffset.y};
   worldFrame.scrollHeight = worldFrame.rectangle.height;

   deleteButton.rectangle = {center.x - worldButtonOffsetX, worldFrame.rectangle.y + worldFrame.rectangle.height + buttonPaddingY, buttonWidth, buttonHeight};
   deleteButton.text = "Delete World";
   deleteButton.disabled = true;
   renameButton.rectangle = {deleteButton.rectangle.x - buttonPaddingX, deleteButton.rectangle.y, buttonWidth, buttonHeight};
   renameButton.text = "Rename World";
   renameButton.disabled = true;
   backButton.rectangle = {renameButton.rectangle.x - buttonPaddingX, renameButton.rectangle.y, buttonWidth, buttonHeight};
   backButton.text = "Back";

   favoriteButton.rectangle = {center.x + worldButtonOffsetX, worldFrame.rectangle.y + worldFrame.rectangle.height + buttonPaddingY, buttonWidth, buttonHeight};
   favoriteButton.text = "Favorite";
   favoriteButton.disabled = true;
   playWorldButton.rectangle = {favoriteButton.rectangle.x + buttonPaddingX, favoriteButton.rectangle.y, buttonWidth, buttonHeight};
   playWorldButton.text = "Play World";
   playWorldButton.disabled = true;
   newButton.rectangle = {playWorldButton.rectangle.x + buttonPaddingX, playWorldButton.rectangle.y, buttonWidth, buttonHeight};
   newButton.text = "New";

   backButton.texture = renameButton.texture = deleteButton.texture = favoriteButton.texture = playWorldButton.texture = newButton.texture = &getTexture("button");
   loadWorldButtons();

   // Init world creation screen
   backButtonCreation.rectangle = deleteButton.rectangle;
   backButtonCreation.text = "Back";
   createButtonCreation.rectangle = favoriteButton.rectangle;
   createButtonCreation.text = "Create";

   worldName.rectangle = {center.x - worldNameSize.x / 2.f, center.y - worldNameSize.y / 2.f, worldNameSize.x, worldNameSize.y};
   worldName.maxChars = maxWorldNameSize;
   shouldWorldBeFlat.rectangle = {center.x - 35.f, worldName.rectangle.y + 200.f, 70.f, 70.f};

   backButtonCreation.texture = createButtonCreation.texture = &getTexture("button");

   // Init world renaming screen
   backButtonRenaming = backButtonCreation;
   renameButtonRenaming = createButtonCreation;
   renameButtonRenaming.text = "Rename";
   renameInput = worldName;
   resetBackground();
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::title:           updateTitle();           break;
   case Phase::levelSelection:  updateLevelSelection();  break;
   case Phase::levelCreation:   updateLevelCreation();   break;
   case Phase::levelRenaming:   updateLevelRenaming();   break;
   case Phase::generatingLevel: updateGeneratingLevel(); break;
   }
}

// Update title

void MenuState::updateTitle() {
   playButton.update();
   optionsButton.update();
   quitButton.update();

   if (playButton.clicked || handleKeyPressWithSound(KEY_ENTER)) {
      phase = Phase::levelSelection;
   }

   if (optionsButton.clicked) {
      insertPopup("Options are a WIP", "Options are not implemented as of now, but they are planned to be soon. Stay tuned for future updates.", false);
   }

   if (quitButton.clicked) {
      fadingOut = true;
   }
}

// Update level selection screen

void MenuState::updateLevelSelection() {
   backButton.update();
   newButton.update();
   worldFrame.update();

   const float offsetY = worldFrame.getOffsetY();
   bool anyClicked = false;
   
   for (Button &button: worldButtons) {
      if (!worldFrame.inFrame(button.normalizeRect())) {
         continue;
      }

      button.update(offsetY);
      if (!button.clicked) {
         continue;
      }

      if (selectedButton) {
         selectedButton->texture = &getTexture("button_long");
      }

      if (selectedButton == &button) {
         selectedWorld = button.text;
         fadingOut = playing = true;
         return;
      }

      selectedButton = &button;
      selectedButton->texture = &getTexture("button_long_selected");
      anySelected = true;
      anyClicked = true;
   }

   if (backButton.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
      phase = Phase::title;
   }

   if (newButton.clicked) {
      phase = Phase::levelCreation;
      worldName.text = generateRandomWorldName();
   }

   // Quick world navigation

   const bool shouldGoDown = isKeyRepeating(KEY_DOWN, downKeyTimer, downKeyDelayTimer);
   const bool shouldGoUp   = isKeyRepeating(KEY_UP, upKeyTimer, upKeyDelayTimer);

   if (!worldButtons.empty() && (shouldGoUp || shouldGoDown)) {
      if (!anySelected) {
         anySelected = true;
         selectedButton = (shouldGoUp ? &worldButtons.back() : &worldButtons.front());
      } else {
         size_t currentIndex = getSelectedButtonIndex();
         currentIndex = (shouldGoUp ? (currentIndex - 1 + worldButtons.size()) : (currentIndex + 1)) % worldButtons.size();
         selectedButton->texture = &getTexture("button_long");
         selectedButton = &worldButtons.at(currentIndex);
      }
      selectedButton->texture = &getTexture("button_long_selected");
   }

   // Update world-specific buttons

   if (anySelected) {
      favoriteButton.text = (selectedButton->favorite ? "Unfavorite" : "Favorite");
   }

   deleteButton.disabled = !anySelected;
   renameButton.disabled = !anySelected;
   favoriteButton.disabled = !anySelected;
   playWorldButton.disabled = !anySelected;

   deleteButton.update();
   renameButton.update();
   favoriteButton.update();
   playWorldButton.update();

   if (deleteButton.clicked) {
      if (selectedButton->favorite) {
         insertPopup("Notice", format("World '{}' cannot be deleted as it is favorited. If you wish to proceed, please unfavorite it and try again.", selectedButton->text), false);
         return;
      }

      insertPopup("Confirmation Request", format("Are you sure that you want to delete world '{}'? You won't be able to recover the world!.", selectedButton->text), true);
      deleteClicked = true;
      return;
   }

   if (deleteClicked && isPopupConfirmed()) {
      std::string fileName = format("data/worlds/{}.txt", selectedButton->text);

      if (!std::filesystem::remove_all(fileName)) {
         insertPopup("Notice", format("World '{}' could not be deleted. File '{}' was{}found. If file was not found, check the 'data/worlds/' folder, if it was, check your permissions.", selectedButton->text, fileName, (std::filesystem::exists(selectedButton->text) ? " " : " not ")), false);
      }
      loadWorldButtons();
   }
   deleteClicked = false;

   if (renameButton.clicked) {
      phase = Phase::levelRenaming;
      wasFavoriteBeforeRenaming = selectedButton->favorite;
      selectedWorld = selectedButton->text;
   }

   if (favoriteButton.clicked) {
      if (selectedButton->favorite) {
         favoriteWorlds.erase(std::remove(favoriteWorlds.begin(), favoriteWorlds.end(), selectedButton->text), favoriteWorlds.end());
      } else {
         favoriteWorlds.push_back(selectedButton->text);
      }

      std::string worldName = selectedButton->text;
      selectedButton->favorite = !selectedButton->favorite;
      
      saveLinesToFile("data/favorites.txt", favoriteWorlds);
      sortWorldButtonsByFavorites();

      for (Button &button: worldButtons) {
         if (button.text == worldName) {
            selectedButton = &button;
         }
      }
   }

   if (playWorldButton.clicked || (anySelected && handleKeyPressWithSound(KEY_ENTER))) {
      selectedWorld = selectedButton->text;
      fadingOut = playing = true;
      return;
   }

   // Ignore renameButton to make it reset after clicking. Required to avoid a graphical glitch
   if (selectedButton && !anyClicked && !deleteButton.clicked && !favoriteButton.clicked && !playWorldButton.clicked && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      selectedButton->texture = &getTexture("button_long");
      selectedButton = nullptr;
      anySelected = false;
   }
}

// Update level creation screen

void MenuState::updateLevelCreation() {
   backButtonCreation.update();
   createButtonCreation.update();
   worldName.update();
   shouldWorldBeFlat.update();

   if (backButtonCreation.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
      phase = Phase::levelSelection;
   }

   if (createButtonCreation.clicked || (!worldName.typing && handleKeyPressWithSound(KEY_ENTER))) {
      // Input characters are capped at maxWorldNameSize already
      if (worldName.text.size() < minWorldNameSize) {
         insertPopup("Invalid World Name", format("World name must contain from {} to {} characters, but it has {} instead.", minWorldNameSize, maxWorldNameSize, worldName.text.size()), false);
         return;
      }

      phase = Phase::generatingLevel;
   }
}

// Update level renaming screen

void MenuState::updateLevelRenaming() {
   backButtonRenaming.update();
   renameButtonRenaming.update();
   renameInput.update();

   if (backButtonRenaming.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
      phase = Phase::levelSelection;
   }

   if (renameButtonRenaming.clicked || (!renameInput.typing && handleKeyPressWithSound(KEY_ENTER))) {
      // Input characters are capped at maxWorldNameSize already
      if (renameInput.text.size() < minWorldNameSize) {
         insertPopup("Invalid World Name", format("World name must contain from {} to {} characters, but it has {} instead.", minWorldNameSize, maxWorldNameSize, renameInput.text.size()), false);
         return;
      }

      std::string newName = format("data/worlds/{}.txt", renameInput.text);
      if (std::filesystem::exists(newName) && std::filesystem::is_regular_file(newName)) {
         insertPopup("Invalid World Name", format("World with the name '{}' already exists.", renameInput.text), false);
         return;
      }

      std::filesystem::rename(format("data/worlds/{}.txt", selectedWorld), newName);

      if (wasFavoriteBeforeRenaming) {
         favoriteWorlds.erase(std::remove(favoriteWorlds.begin(), favoriteWorlds.end(), renameInput.text), favoriteWorlds.end());
         favoriteWorlds.push_back(renameInput.text);
         saveLinesToFile("data/favorites.txt", favoriteWorlds);
      }

      loadWorldButtons();
      renameInput.text.clear();
      phase = Phase::levelSelection;
   }
}

// Update level generation screen

void MenuState::updateGeneratingLevel() {
   MapGenerator generator (worldName.text, defaultMapSizeX, defaultMapSizeY, shouldWorldBeFlat.checked);
   generator.generate();

   loadWorldButtons();
   phase = Phase::levelSelection;
}

// Render

void MenuState::render() const {
   drawBackground(foregroundTexture, backgroundTexture, parallaxBgSpeed, parallaxFgSpeed, menuSunSpeed);

   switch (phase) {
   case Phase::title:           renderTitle();           break;
   case Phase::levelSelection:  renderLevelSelection();  break;
   case Phase::levelCreation:   renderLevelCreation();   break;
   case Phase::levelRenaming:   renderLevelRenaming();   break;
   case Phase::generatingLevel: renderGeneratingLevel(); break;
   }
}

// Render title

void MenuState::renderTitle() const {
   drawText(getScreenCenter({0.f, titleOffsetX}), "SANDBOX 2D", 180);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

// Render level selection screen

void MenuState::renderLevelSelection() const {
   drawText(getScreenCenter({0.f, titleOffsetX2}), "SELECT WORLD", 180);
   backButton.render();
   renameButton.render();
   deleteButton.render();
   favoriteButton.render();
   playWorldButton.render();
   newButton.render();
   worldFrame.render();

   const float offsetY = worldFrame.getOffsetY();
   for (const Button &button: worldButtons) {
      if (!worldFrame.inFrame(button.normalizeRect())) {
         continue;
      }

      button.render(offsetY);
      if (!button.favorite) {
         continue;
      }

      Vector2 position = {button.rectangle.x + (button.rectangle.width * button.scale) / 2.f - (button.rectangle.height * button.scale) / 2.f, button.rectangle.y - offsetY};
      drawTexture(getTexture("star"), position, {worldStarSize * button.scale, worldStarSize * button.scale});
   }
}

// Render level creation screen

void MenuState::renderLevelCreation() const {
   drawText(getScreenCenter({0.f, titleOffsetX2}), "CREATE WORLD", 180);
   backButtonCreation.render();
   createButtonCreation.render();
   worldName.render();
   shouldWorldBeFlat.render();
   drawText({worldName.rectangle.x - 125.f, worldName.rectangle.y + worldName.rectangle.height / 2.f}, "World Name:", 50);
   drawText({worldName.rectangle.x - 125.f, shouldWorldBeFlat.rectangle.y + shouldWorldBeFlat.rectangle.height / 2.f}, "Flat World:", 50);
}

// Render level renaming screen

void MenuState::renderLevelRenaming() const {
   drawText(getScreenCenter({0.0f, titleOffsetX2}), "RENAME WORLD", 180);
   backButtonRenaming.render();
   renameButtonRenaming.render();
   renameInput.render();

   drawText({renameInput.rectangle.x - 175.0f, renameInput.rectangle.y + renameInput.rectangle.height / 2.0f}, "New World Name:", 50);
   drawText({renameInput.rectangle.x - 175.0f, renameInput.rectangle.y + 235.0f}, "Old World Name:", 50);
   drawText({renameInput.rectangle.x + renameInput.rectangle.width / 2.0f, renameInput.rectangle.y + 235.0f}, selectedWorld.c_str(), 50);
}

// Render level generation screen

void MenuState::renderGeneratingLevel() const {
   drawText(getScreenCenter(), (std::string("Generating World '") + worldName.text + "'...").c_str(), 50);
}

// Change states

State* MenuState::change() {
   if (playing) {
      return new GameState(selectedWorld);
   }
   return nullptr; // Quit the game
}

// World selection functions

void MenuState::loadWorldButtons() {
   favoriteWorlds = getAllLinesFromFile("data/favorites.txt");
   std::filesystem::create_directories("data/worlds/");

   worldButtons.clear();
   for (const auto &file: std::filesystem::directory_iterator("data/worlds")) {
      Button button;
      button.text = file.path().stem().string();
      button.texture = &getTexture("button_long");
      button.favorite = isWorldFavorite(button.text);
      worldButtons.push_back(button);
   }
   sortWorldButtonsByFavorites();
}

std::string MenuState::generateRandomWorldName() const {
   const std::string adjective = getRandomLineFromFile("assets/adjectives.txt");
   const std::string noun      = getRandomLineFromFile("assets/nouns.txt");
   return adjective + " " + noun;
}

bool MenuState::isWorldFavorite(const std::string &name) const {
   for (const std::string &world: favoriteWorlds) {
      if (world == name) {
         return true;
      }
   }
   return false;
}

void MenuState::sortWorldButtonsByFavorites() {
   std::sort(worldButtons.begin(), worldButtons.end(), [](Button &a, Button &b) -> bool {
      if (a.favorite != b.favorite) {
         return a.favorite && !b.favorite;
      }
      return a.text < b.text;
   });

   size_t index = 0;
   for (Button &button: worldButtons) {
      button.rectangle = {360.f - scrollBarWidth / 2.f, 210.f + 110.f * index, worldFrame.rectangle.width - 120.f - scrollBarWidth / 2.f, 100.f};
      button.rectangle.x += button.rectangle.width / 2.f;
      button.rectangle.y += button.rectangle.height / 2.f;

      worldFrame.scrollHeight = std::max(worldFrame.rectangle.height, button.rectangle.y + button.rectangle.height / 2.f);
      index += 1;
   }
}

// Helper functions

bool MenuState::isKeyRepeating(int key, float &repeatTimer, float &delayTimer) {
   const bool pressed = IsKeyPressed(key);
   const bool down    = IsKeyDown(key);

   if (!down) {
      repeatTimer = 0.0f;
      delayTimer = 0.0f;
      return false; // For a key to be pressed, it must be down first
   }

   if (delayTimer < worldSelectionKeyStartDelay) {
      delayTimer += GetFrameTime();
      return pressed;
   }

   repeatTimer += GetFrameTime();
   if (repeatTimer >= worldSelectionKeyDelay) {
      repeatTimer = 0.0f;
      return true;
   }
   return pressed;
}

size_t MenuState::getSelectedButtonIndex() const {
   size_t index = 0;
   for (const Button &button: worldButtons) {
      if (selectedButton == &button) {
         break;
      }
      index += 1;
   }
   return index;
}
