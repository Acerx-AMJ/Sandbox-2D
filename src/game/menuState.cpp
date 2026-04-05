#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/input.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "ui/popup.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <filesystem>
#include <thread>

// Constants

constexpr int maxWorldNameSize  = 48;
constexpr int minWorldNameSize  = 3;

constexpr float worldSelectionKeyDelay      = 0.125f;
constexpr float worldSelectionKeyStartDelay = 0.333f;

constexpr int defaultMapSizeX = 2000;
constexpr int defaultMapSizeY = 750;

// Constructors

MenuState::MenuState() {
   // Init title screen
   playButton.text = "Play";
   optionsButton.text = "Options";
   optionsButton.keybind = "O";
   quitButton.text = "Quit";
   playButton.texture = optionsButton.texture = quitButton.texture = &getTexture("button");

   // Init world selection screen

   worldSearchBar.maxChars = maxWorldNameSize;
   worldSearchBar.fallback = "Search for a World...";
   worldSearchBar.texture = &getTexture("search_bar");
   deleteButton.text = "Delete World";
   deleteButton.keybind = "D";
   deleteButton.disabled = true;
   renameButton.text = "Rename World";
   renameButton.keybind = "R";
   renameButton.disabled = true;
   backButton.text = "Back";
   favoriteButton.text = "Favorite";
   favoriteButton.keybind = "F";
   favoriteButton.disabled = true;
   playWorldButton.text = "Play World";
   playWorldButton.disabled = true;
   newButton.text = "New";
   newButton.keybind = "N";
   backButton.texture = renameButton.texture = deleteButton.texture = favoriteButton.texture = playWorldButton.texture = newButton.texture = &getTexture("button");
   loadWorldButtons();

   // Init world creation screen
   backButtonCreation.text = "Back";
   createButtonCreation.text = "Create";
   worldName.maxChars = maxWorldNameSize;
   worldName.fallback = "Name Your New World...";
   shouldWorldBeFlat.keybind = "F";
   backButtonCreation.texture = createButtonCreation.texture = worldName.texture = &getTexture("button");

   // Init world renaming screen
   backButtonRenaming = backButtonCreation;
   renameButtonRenaming = createButtonCreation;
   renameButtonRenaming.text = "Rename";
   renameInput = worldName;
   renameInput.fallback = "Rename Your World...";

   // Init world generation screen
   generationProgressBar.progress = generationProgressBar.progressInterpolation = 0.0f;
   generationProgressBar.texture = &getTexture("bar");
   generationProgressBar.backgroundTint = GRAY;
   generationProgressBar.foregroundTint = WHITE;

   updateResponsiveness();
   setCurrentBackgroundBiome(MapGenerator::Biome(random(0, (int)MapGenerator::Biome::count - 1)));
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

void MenuState::fixedUpdate() {
   // Menu state does not require any physics
}

void MenuState::updateResponsiveness() {
   const Vector2 center = getScreenCenter();

   float wr = getWidthRatio();
   float hr = getHeightRatio();

   float btnWidth = buttonWidth * wr;
   float btnHeight = buttonHeight * hr;   
   float btnPadY = buttonPaddingY * hr;
   float btnPadX = buttonPaddingX * wr;

   // title screen
   playButton.rectangle = {center.x, center.y, btnWidth, btnHeight};
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + btnPadY, btnWidth, btnHeight};
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + btnPadY, btnWidth, btnHeight};

   // world selection screen
   worldFrame.rectangle = {280.0f * wr, 252.0f * hr, GetScreenWidth() - 600.0f * wr, GetScreenHeight() - 412.0f * hr};
   worldFrame.scrollHeight = worldFrame.rectangle.height;
   worldFrame.scrollbarY = worldFrame.rectangle.y;
   worldSearchBar.rectangle = {worldFrame.rectangle.x + worldFrame.rectangle.width / 2.0f, worldFrame.rectangle.y - 45.0f * hr, worldFrame.rectangle.width, 73.333f * hr};
   deleteButton.rectangle = {center.x - 120.0f * wr, worldFrame.rectangle.y + worldFrame.rectangle.height + btnPadY, btnWidth, btnHeight};
   renameButton.rectangle = {deleteButton.rectangle.x - btnPadX, deleteButton.rectangle.y, btnWidth, btnHeight};
   backButton.rectangle = {renameButton.rectangle.x - btnPadX, renameButton.rectangle.y, btnWidth, btnHeight};
   favoriteButton.rectangle = {center.x + 120.0f * wr, worldFrame.rectangle.y + worldFrame.rectangle.height + btnPadY, btnWidth, btnHeight};
   playWorldButton.rectangle = {favoriteButton.rectangle.x + btnPadX, favoriteButton.rectangle.y, btnWidth, btnHeight};
   newButton.rectangle = {playWorldButton.rectangle.x + btnPadX, playWorldButton.rectangle.y, btnWidth, btnHeight};

   size_t index = 0;
   for (Button &button: worldButtons) {
      button.rectangle = {360.f * wr - (scrollBarWidth * wr) / 2.f, 262.f * hr + 110.f * hr * index, worldFrame.rectangle.width - 120.f * wr - (scrollBarWidth * wr) / 2.f, 100.f * hr};
      button.rectangle.x += button.rectangle.width / 2.f;
      button.rectangle.y += button.rectangle.height / 2.f;

      worldFrame.scrollHeight = std::max(worldFrame.rectangle.height, button.rectangle.y + button.rectangle.height / 2.f);
      index += 1;
   }

   // world creation screen
   backButtonCreation.rectangle = deleteButton.rectangle;
   createButtonCreation.rectangle = favoriteButton.rectangle;
   worldName.rectangle = {center.x, center.y, 420.0f * wr, 140.0f * hr};
   shouldWorldBeFlat.rectangle = {center.x - 35.f * wr, worldName.rectangle.y + 100.f * hr, 70.f * wr, 70.f * hr};

   // world renaming screen
   backButtonRenaming.rectangle = backButtonCreation.rectangle;
   renameButtonRenaming.rectangle = createButtonCreation.rectangle;
   renameInput.rectangle = worldName.rectangle;

   // world generation screen
   generationProgressBar.rectangle = {center.x, center.y, 18.0f * 50.0f * wr, 50.0f * hr};
}

// Update title

void MenuState::updateTitle() {
   playButton.update(dt);
   optionsButton.update(dt);
   quitButton.update(dt);

   if (playButton.clicked || handleKeyPressWithSound(KEY_ENTER)) {
      phase = Phase::levelSelection;
   }

   if (optionsButton.clicked || handleKeyPressWithSound(KEY_O)) {
      insertPopup("Options are a WIP", "Options are not implemented as of now, but they are planned to be soon. Stay tuned for future updates.", PopupType::info);
   }

   if (quitButton.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

// Update level selection screen

void MenuState::updateLevelSelection() {
   backButton.update(dt);
   newButton.update(dt);
   worldFrame.update(dt);
   worldSearchBar.update(dt);

   if (backButton.clicked || (!worldSearchBar.typing && handleKeyPressWithSound(KEY_ESCAPE))) {
      worldSearchBar.typing = false;
      phase = Phase::title;
   }

   if (newButton.clicked || (!worldSearchBar.typing && handleKeyPressWithSound(KEY_N))) {
      worldSearchBar.typing = false;
      phase = Phase::levelCreation;
      worldName.text = generateRandomWorldName();
   }

   if (handleKeyPressWithSound(KEY_TAB)) {
      worldSearchBar.typing = !worldSearchBar.typing;
      if (worldSearchBar.typing) {
         worldSearchBar.text.clear();
         worldSearchBar.changed = true;
      }
   }

   if (worldSearchBar.changed) {
      resetSelection();
      loadWorldButtons();
   }

   const float offsetY = worldFrame.getOffsetY();
   bool wantsToPlay = false;

   for (Button &button: worldButtons) {
      if (!worldFrame.inFrame(button.normalizeRect())) {
         continue;
      }

      button.update(dt, offsetY);
      if (!button.clicked) {
         continue;
      }

      if (selectedButton == &button) {
         wantsToPlay = true;
         break;
      }
      selectButton(button);
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
      worldFrame.setProgressBasedOnPosition(selectedButton->rectangle.y - selectedButton->rectangle.height / 2.0f);
   }

   // Update world-specific buttons

   if (anySelected) {
      favoriteButton.text = (selectedButton->favorite ? "Unfavorite" : "Favorite");
   }

   deleteButton.disabled = !anySelected;
   renameButton.disabled = !anySelected;
   favoriteButton.disabled = !anySelected;
   playWorldButton.disabled = !anySelected;

   deleteButton.update(dt);
   renameButton.update(dt);
   favoriteButton.update(dt);
   playWorldButton.update(dt);

   if (IsKeyDown(KEY_LEFT_SHIFT) && !worldButtons.empty()) {
      deleteButton.text = "Delete All Worlds";
      deleteButton.disabled = false;
   } else {
      deleteButton.text = "Delete World";
   }

   if (deleteButton.clicked || (!deleteButton.disabled && handleKeyPressWithSound(KEY_D))) {
      // Delete all worlds instead
      if (IsKeyDown(KEY_LEFT_SHIFT)) {
         int deletableWorldCount = worldButtons.size() - favoriteWorlds.size();
         if (deletableWorldCount == 0) {
            return;
         }

         insertPopup("Confirmation Request", format("Are you sure that you want to delete all non-favorited worlds? This includes {} worlds. You won't be able to recover any of them!", deletableWorldCount), PopupType::confirmation);
         megaDeleteClicked = true;
         return;
      }
      
      if (selectedButton->favorite) {
         insertPopup("Notice", format("World '{}' cannot be deleted as it is favorited. If you wish to proceed, please unfavorite it and try again.", selectedButton->text), PopupType::info);
         return;
      }

      insertPopup("Confirmation Request", format("Are you sure that you want to delete world '{}'? You won't be able to recover it!", selectedButton->text), PopupType::confirmation);
      deleteClicked = true;
      return;
   }

   if (megaDeleteClicked && isPopupConfirmed()) {
      int failedCount = 0;
      for (const Button &button: worldButtons) {
         if (isWorldFavorite(button.text)) {
            continue;
         }
         std::string fileName = format("data/worlds/{}.bin", button.text);

         if (!std::filesystem::remove_all(fileName)) {
            failedCount += 1;
         }
      }

      if (failedCount != 0) {
         insertPopup("Notice", format("{} worlds could not be deleted. Please check the 'data/worlds/' folder, if the files are present, check your permissions.", failedCount), PopupType::error);
      }
      loadWorldButtons();
   }
   megaDeleteClicked = false;

   if (deleteClicked && isPopupConfirmed()) {
      std::string fileName = format("data/worlds/{}.bin", selectedButton->text);

      if (!std::filesystem::remove_all(fileName)) {
         insertPopup("Notice", format("World '{}' could not be deleted. Please check the 'data/worlds/' folder, if the file is present, check your permissions.", selectedButton->text, fileName, (std::filesystem::exists(selectedButton->text) ? " " : " not ")), PopupType::error);
      }
      loadWorldButtons();
   }
   deleteClicked = false;

   if (renameButton.clicked || (!renameButton.disabled && handleKeyPressWithSound(KEY_R))) {
      worldSearchBar.typing = false;
      
      phase = Phase::levelRenaming;
      wasFavoriteBeforeRenaming = selectedButton->favorite;
      selectedWorld = selectedButton->text;
   }

   if (favoriteButton.clicked || (!favoriteButton.disabled && handleKeyPressWithSound(KEY_F))) {
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

   if (wantsToPlay || playWorldButton.clicked || (anySelected && handleKeyPressWithSound(KEY_ENTER))) {
      selectedWorld = selectedButton->text;

      if (getLatestVersion() != getFileVersion(selectedWorld)) {
         invalidVersionClicked = true;
         resetSelection();
         insertPopup("Confirmation Request", format("World '{}' uses an outdated file version. The latest version is {}, whereas its version is {}. Are you sure that you want to continue? Your world might get corrupted and become unrecoverable!", selectedWorld, getLatestVersion(), getFileVersion(selectedWorld)), PopupType::confirmation);
         return;
      }

      fadingOut = playing = true;
      return;
   }

   if (invalidVersionClicked && isPopupConfirmed()) {
      fadingOut = playing = true; // User confirmed to corrupt their world
      return;
   }
   invalidVersionClicked = false;

   if (anySelected && isMousePressedOutsideUI(MOUSE_BUTTON_LEFT)) {
      resetSelection();
   }
}

// Update level creation screen

void MenuState::updateLevelCreation() {
   backButtonCreation.update(dt);
   createButtonCreation.update(dt);
   worldName.update(dt);
   shouldWorldBeFlat.update();

   if (backButtonCreation.clicked || (!worldName.typing && handleKeyPressWithSound(KEY_ESCAPE))) {
      phase = Phase::levelSelection;
   }

   if (handleKeyPressWithSound(KEY_TAB)) {
      worldName.typing = !worldName.typing;
      if (worldName.typing) {
         worldName.text.clear();
      }
   }

   if (!worldName.typing && handleKeyPressWithSound(KEY_F)) {
      shouldWorldBeFlat.checked = !shouldWorldBeFlat.checked;
   }

   if (createButtonCreation.clicked || (!worldName.typing && handleKeyPressWithSound(KEY_ENTER))) {
      // Input characters are capped at maxWorldNameSize already
      if (worldName.text.size() < minWorldNameSize) {
         insertPopup("Invalid World Name", format("World name must contain from {} to {} characters, but it has {} instead.", minWorldNameSize, maxWorldNameSize, worldName.text.size()), PopupType::error);
         return;
      }

      // World with the same name already exists
      for (const Button &button: worldButtons) {
         if (button.text == worldName.text) {
            insertPopup("World Exists", format("Cannot create world with the name '{}', as a world with the same name already exists.", worldName.text), PopupType::error);
            return;
         }
      }

      generationSplash = getRandomLineFromFile("assets/splash.txt");
      wrapText(generationSplash, GetScreenWidth() - 50.0f, 40.0f, 1.0f);

      worldName.typing = false;
      phase = Phase::generatingLevel;
   }
}

// Update level renaming screen

void MenuState::updateLevelRenaming() {
   backButtonRenaming.update(dt);
   renameButtonRenaming.update(dt);
   renameInput.update(dt);

   if (backButtonRenaming.clicked || (!renameInput.typing && handleKeyPressWithSound(KEY_ESCAPE))) {
      phase = Phase::levelSelection;
   }

   if (handleKeyPressWithSound(KEY_TAB)) {
      renameInput.typing = !renameInput.typing;
      if (renameInput.typing) {
         renameInput.text.clear();
      }
   }

   if (renameButtonRenaming.clicked || (!renameInput.typing && handleKeyPressWithSound(KEY_ENTER))) {
      // Input characters are capped at maxWorldNameSize already
      if (renameInput.text.size() < minWorldNameSize) {
         insertPopup("Invalid World Name", format("World name must contain from {} to {} characters, but it has {} instead.", minWorldNameSize, maxWorldNameSize, renameInput.text.size()), PopupType::error);
         return;
      }

      // World with the same name already exists
      for (const Button &button: worldButtons) {
         if (button.text == renameInput.text) {
            insertPopup("World Exists", format("Cannot rename world to '{}', as a world with the same name already exists.", renameInput.text), PopupType::error);
            return;
         }
      }

      std::string newName = format("data/worlds/{}.bin", renameInput.text);
      if (std::filesystem::exists(newName) && std::filesystem::is_regular_file(newName)) {
         insertPopup("Invalid World Name", format("World with the name '{}' already exists.", renameInput.text), PopupType::error);
         return;
      }

      std::filesystem::rename(format("data/worlds/{}.bin", selectedWorld), newName);

      if (wasFavoriteBeforeRenaming) {
         favoriteWorlds.erase(std::remove(favoriteWorlds.begin(), favoriteWorlds.end(), renameInput.text), favoriteWorlds.end());
         favoriteWorlds.push_back(renameInput.text);
         saveLinesToFile("data/favorites.txt", favoriteWorlds);
      }

      loadWorldButtons();
      renameInput.text.clear();
      renameInput.typing = false;
      phase = Phase::levelSelection;
   }
}

// Update level generation screen

void MenuState::updateGeneratingLevel() {
   if (generatedWorld) {
      generatedWorld = false;
      generationProgressBar.progressInterpolation = generationProgressBar.progress = 0.0f;

      generator = new MapGenerator(worldName.text, defaultMapSizeX, defaultMapSizeY, shouldWorldBeFlat.checked, generationInfoTextMutex, generationInfoText, generationProgressBar.progress);
      std::thread thread(&MapGenerator::generate, generator);
      thread.detach();
   }
   generationProgressBar.update(dt);

   if (generator && generator->isCompleted) {
      delete generator;
      loadWorldButtons();
      phase = Phase::levelSelection;
      generatedWorld = true;
   }
}

// Render

void MenuState::render() {
   drawBackground(dt, dt, 15.0f * dt);

   switch (phase) {
   case Phase::title:           renderTitle();           break;
   case Phase::levelSelection:  renderLevelSelection();  break;
   case Phase::levelCreation:   renderLevelCreation();   break;
   case Phase::levelRenaming:   renderLevelRenaming();   break;
   case Phase::generatingLevel: renderGeneratingLevel(); break;
   }
}

// Render title

void MenuState::renderTitle() {
   drawText(getScreenCenter({0.f, -200.0f * getHeightRatio()}), "SANDBOX 2D", getFontSize(180));
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

// Render level selection screen

void MenuState::renderLevelSelection() {
   drawText(getScreenCenter({0.f, -425.0f * getHeightRatio()}), "SELECT WORLD", getFontSize(180));
   backButton.render();
   renameButton.render();
   deleteButton.render();
   favoriteButton.render();
   playWorldButton.render();
   newButton.render();
   worldFrame.render();
   worldSearchBar.render();

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
      drawTexture(getTexture("star"), position, applyCubicResponsiveness({50.0f * button.scale, 50.0f * button.scale}));
   }
}

// Render level creation screen

void MenuState::renderLevelCreation() {
   drawText(getScreenCenter({0.f, -400.0f * getHeightRatio()}), "CREATE WORLD", getFontSize(180));
   backButtonCreation.render();
   createButtonCreation.render();
   worldName.render();
   shouldWorldBeFlat.render();
   drawText({worldName.rectangle.x - worldName.rectangle.width / 2.0f - 125.0f * getWidthRatio(), worldName.rectangle.y}, "World Name:", getFontSize(50));
   drawText({worldName.rectangle.x - worldName.rectangle.width / 2.0f - 125.0f * getWidthRatio(), shouldWorldBeFlat.rectangle.y + shouldWorldBeFlat.rectangle.height / 2.f}, "Flat World:", getFontSize(50));
}

// Render level renaming screen

void MenuState::renderLevelRenaming() {
   drawText(getScreenCenter({0.0f, -400.0f * getHeightRatio()}), "RENAME WORLD", getFontSize(180));
   backButtonRenaming.render();
   renameButtonRenaming.render();
   renameInput.render();

   drawText({renameInput.rectangle.x - renameInput.rectangle.width / 2.0f - 175.0f * getWidthRatio(), renameInput.rectangle.y}, "New World Name:", getFontSize(50));
   drawText({renameInput.rectangle.x - renameInput.rectangle.width / 2.0f - 175.0f * getWidthRatio(), renameInput.rectangle.y + 150.0f * getHeightRatio()}, "Old World Name:", getFontSize(50));
   drawText({renameInput.rectangle.x, renameInput.rectangle.y + 150.0f * getHeightRatio()}, selectedWorld.c_str(), getFontSize(50));
}

// Render level generation screen

void MenuState::renderGeneratingLevel() {
   generationProgressBar.render();
   drawText(getScreenCenter({0.0f, getHeightRatio() * -100.0f}), generationInfoText.c_str(), getFontSize(50.0f));
   drawText(getScreenCenter({0.0f, getHeightRatio() * 100.0f}), generationSplash.c_str(), getFontSize(40.0f));
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
   if (anySelected) {
      anySelected = false;
      selectedButton = nullptr;
   }

   for (const auto &file: std::filesystem::directory_iterator("data/worlds")) {
      Button button;
      button.text = file.path().stem().string();
      button.texture = &getTexture("button_long");
      button.favorite = isWorldFavorite(button.text);

      if (worldSearchBar.text.empty()) {
         worldButtons.push_back(button);
         continue;
      }

      std::string copyA = button.text;
      std::string copyB = worldSearchBar.text;
      std::transform(copyA.begin(), copyA.end(), copyA.begin(), ::tolower);
      std::transform(copyB.begin(), copyB.end(), copyB.begin(), ::tolower);

      if (copyA.find(copyB) != std::string::npos) {
         worldButtons.push_back(button);
      }
   }
   sortWorldButtonsByFavorites();
}

void MenuState::sortWorldButtonsByFavorites() {
   std::sort(worldButtons.begin(), worldButtons.end(), [](Button &a, Button &b) -> bool {
      if (a.favorite != b.favorite) {
         return a.favorite && !b.favorite;
      }

      std::string copyA = a.text;
      std::string copyB = b.text;
      std::transform(copyA.begin(), copyA.end(), copyA.begin(), ::tolower);
      std::transform(copyB.begin(), copyB.end(), copyB.begin(), ::tolower);
      return copyA < copyB;
   });

   float wr = getWidthRatio();
   float hr = getHeightRatio();

   size_t index = 0;
   for (Button &button: worldButtons) {
      button.rectangle = {360.f * wr - (scrollBarWidth * wr) / 2.f, 262.f * hr + 110.f * hr * index, worldFrame.rectangle.width - 120.f * wr - (scrollBarWidth * wr) / 2.f, 100.f * hr};
      button.rectangle.x += button.rectangle.width / 2.f;
      button.rectangle.y += button.rectangle.height / 2.f;

      worldFrame.scrollHeight = std::max(worldFrame.rectangle.height, button.rectangle.y + button.rectangle.height / 2.f);
      index += 1;
   }
}

void MenuState::resetSelection() {
   if (anySelected) {
      anySelected = false;
      selectedButton->texture = &getTexture("button_long");
      selectedButton = nullptr;
   }
}

void MenuState::selectButton(Button &button) {
   if (anySelected) {
      selectedButton->texture = &getTexture("button_long");
   }
   anySelected = true;
   selectedButton = &button;
   selectedButton->texture = &getTexture("button_long_selected");
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

// Helper functions

bool MenuState::isKeyRepeating(int key, float &repeatTimer, float &delayTimer) {
   const bool pressed = isKeyPressed(key);
   const bool down    = isKeyDown(key);

   if (!down) {
      repeatTimer = 0.0f;
      delayTimer = 0.0f;
      return false; // For a key to be pressed, it must be down first
   }

   if (delayTimer < worldSelectionKeyStartDelay) {
      delayTimer += realDt;
      return pressed;
   }

   repeatTimer += realDt;
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
