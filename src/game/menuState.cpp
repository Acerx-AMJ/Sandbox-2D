#include "SRU/util.hpp"
#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/input.hpp"
#include "mngr/fileio.hpp"
#include "objs/generation.hpp"
#include "objs/parallax.hpp"
#include "ui/popup.hpp"
#include "SRU/assets.hpp"
#include "SRU/file.hpp"
#include "SRU/random.hpp"
#include "SRU/render.hpp"
#include "SRU/text.hpp"
#include <filesystem>
#include <thread>

// Constants

constexpr int maxWorldNameSize = 48;
constexpr int minWorldNameSize = 3;

constexpr float worldSelectionKeyDelay = 0.125f;
constexpr float worldSelectionKeyStartDelay = 0.333f;

constexpr int defaultMapSizeX = 2000;
constexpr int defaultMapSizeY = 750;

// Constructors

MenuState::MenuState() {
   font = getFont("andy");
   buttonTexture = getTexture("button");
   searchBarTexture = getTexture("search_bar");
   barTexture = getTexture("bar");
   titleTexture = getTexture("title");
   starTexture = getTexture("star");
   scrollframeTexture = getTexture("scrollframe");
   scrollbarTexture = getTexture("scrollbar");
   longButtonTexture = getTexture("button_long");
   longSelectedButtonTexture = getTexture("button_long_selected");

   // Init title screen
   playButton.init(font, buttonTexture, CENTER, "Play");
   optionsButton.init(font, buttonTexture, CENTER, "Options", "O");
   quitButton.init(font, buttonTexture, CENTER, "Quit");

   // Init world selection screen
   worldSearchBar.init(font, searchBarTexture, TOP_LEFT, maxWorldNameSize, "Search for a World...");
   deleteButton.init(font, buttonTexture, CENTER, "Delete World", "D");
   renameButton.init(font, buttonTexture, CENTER, "Rename World", "F2");
   backButton.init(font, buttonTexture, CENTER, "Back");
   favoriteButton.init(font, buttonTexture, CENTER, "Favorite", "F");
   playWorldButton.init(font, buttonTexture, CENTER, "Play World");
   newButton.init(font, buttonTexture, CENTER, "New", "N");
   loadWorldButtons();

   // Init world creation screen
   backButtonCreation.init(font, buttonTexture, CENTER, "Back");
   createButtonCreation.init(font, buttonTexture, CENTER, "Create");
   worldName.init(font, buttonTexture, CENTER, maxWorldNameSize, "Name Your New World...");
   shouldWorldBeFlat.init(font, CENTER, "F");

   // Init world renaming screen
   backButtonRenaming.init(font, buttonTexture, CENTER, "Back");
   renameButtonRenaming.init(font, buttonTexture, CENTER, "Rename");
   renameInput.init(font, buttonTexture, CENTER, maxWorldNameSize, "Rename Your World...");

   // Init world generation screen
   generationProgressBar.progress = generationProgressBar.progressInterpolation = 0.0f;
   generationProgressBar.init(barTexture, CENTER, WHITE, GRAY);

   updateResponsiveness();
   setCurrentBackgroundBiome(MapGenerator::Biome(randomInt(0, (int)MapGenerator::Biome::count - 1)));
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::title:           updateTitle(); break;
   case Phase::levelSelection:  updateLevelSelection(); break;
   case Phase::levelCreation:   updateLevelCreation(); break;
   case Phase::levelRenaming:   updateLevelRenaming(); break;
   case Phase::generatingLevel: updateGeneratingLevel(); break;
   }
}

void MenuState::updateResponsiveness() {
   Vector2 bsize = mapRatioToArea(buttonSize, WINDOW_AREA, CUBIC_RATIO);
   Vector2 padding = convertRatio(buttonPadding, CUBIC_RATIO, RATIO);
   float worldButtonStartY = 0.58f - convertRatioY(0.619f / 2.0f, CUBIC_RATIO, RATIO) + padding.y * 0.6f;

   // title screen
   playButton.rect = R4(mapRatioToArea(0.5f, 0.5f), bsize);
   optionsButton.rect = R4(mapRatioToArea(0.5f, 0.5f + padding.y), bsize);
   quitButton.rect = R4(mapRatioToArea(0.5f, 0.5f + padding.y * 2.0f), bsize);

   // world selection screen
   deleteButton.rect = R4(mapRatioToArea(0.5f - padding.x * 0.5f, 0.95f), bsize);
   renameButton.rect = R4(mapRatioToArea(0.5f - padding.x * 1.5f, 0.95f), bsize);
   backButton.rect = R4(mapRatioToArea(0.5f - padding.x * 2.5f, 0.95f), bsize);
   favoriteButton.rect = R4(mapRatioToArea(0.5f + padding.x * 0.5f, 0.95f), bsize);
   playWorldButton.rect = R4(mapRatioToArea(0.5f + padding.x * 1.5f, 0.95f), bsize);
   newButton.rect = R4(mapRatioToArea(0.5f + padding.x * 2.5f, 0.95f), bsize);

   worldSearchBar.rect = mapRatioToArea(R4(0.5f, 0.225f, 1.222f, 0.0679f), CENTER, WINDOW_AREA, CUBIC_RATIO);
   worldFrame = mapRatioToArea(R4(0.5f, 0.575f, 1.222f, 0.619f), CENTER, WINDOW_AREA, CUBIC_RATIO);

   for (int i = 0; i < buttonsInWorldFrame; ++i) {
      worldFrameButtonRects[i] = mapRatioToArea(R4(0.5f, worldButtonStartY + padding.y * i, 1.0f, buttonSize.y), CENTER, WINDOW_AREA, CUBIC_RATIO);
      worldFrameButtonRects[i].x += worldFrameButtonRects[i].width / 2.0f;
      worldFrameButtonRects[i].y += worldFrameButtonRects[i].height / 2.0f;
   }

   // world creation screen
   backButtonCreation.rect = deleteButton.rect;
   createButtonCreation.rect = favoriteButton.rect;
   worldName.rect = mapRatioToArea(R4(0.5f, 0.5f, 0.389f, 0.130f), TOP_LEFT, WINDOW_AREA, CUBIC_RATIO);
   shouldWorldBeFlat.rect = mapRatioToArea(R4(0.5f, 0.6f, 0.064f, 0.064f), TOP_LEFT, WINDOW_AREA, CUBIC_RATIO);

   // world renaming screen
   backButtonRenaming.rect = backButtonCreation.rect;
   renameButtonRenaming.rect = createButtonCreation.rect;
   renameInput.rect = worldName.rect;

   // world generation screen
   generationProgressBar.rect = mapRatioToArea(R4(0.5f, 0.5f, 0.833f, 0.0463f), TOP_LEFT, WINDOW_AREA, CUBIC_RATIO);
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

   bool wantsToPlay = false;
   Vector2 padding = convertRatio(buttonPadding, CUBIC_RATIO, RATIO);
   float startY = 0.58f - convertRatioY(0.619f / 2.0f, CUBIC_RATIO, RATIO) + padding.y * 0.6f;
   size_t index = 0;

   for (Button &button: worldButtons) {
      button.rect = mapRatioToArea(R4(0.5f, startY + padding.y * index, 1.0f, buttonSize.y), CENTER, WINDOW_AREA, CUBIC_RATIO);
      button.rect.x += button.rect.width / 2.0f;
      button.rect.y += button.rect.height / 2.0f;
      index += 1;
   }

   for (int i = scrollIndex; i - scrollIndex < buttonsInWorldFrame && i - scrollIndex < (int)worldButtons.size(); ++i) {
      Button &button = worldButtons[i];
      button.rect = worldFrameButtonRects[i - scrollIndex];
      button.update(dt);

      if (button.clicked) {
         if (selectedButton == &button) {
            wantsToPlay = true;
            break;
         }
         else {
            selectButton(button);
         }
      }
   }

   // Quick world navigation

   bool shouldGoDown = isKeyRepeating(KEY_DOWN, downKeyTimer, downKeyDelayTimer);
   bool shouldGoUp = isKeyRepeating(KEY_UP, upKeyTimer, upKeyDelayTimer);

   float scroll = GetMouseWheelMove();
   if (scroll >= 0.5f) shouldGoUp = true;
   else if (scroll <= -0.5f) shouldGoDown = true;

   if (!worldButtons.empty() && (shouldGoUp || shouldGoDown)) {
      if (!anySelected) {
         anySelected = true;
         selectedButton = (shouldGoUp ? &worldButtons.back() : &worldButtons.front());
         scrollIndex = (shouldGoUp ? fmax(0, (int)worldButtons.size() - buttonsInWorldFrame) : 0);
      } else {
         int currentIndex = getSelectedButtonIndex();
         currentIndex = (shouldGoUp ? (currentIndex - 1 + worldButtons.size()) : (currentIndex + 1)) % worldButtons.size();
         selectedButton->texture = longButtonTexture;
         selectedButton = &worldButtons.at(currentIndex);
         scrollIndex = fmin(currentIndex, fmax(0, (int)worldButtons.size() - buttonsInWorldFrame));
      }
      selectedButton->texture = longSelectedButtonTexture;
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

         insertPopup("Confirmation Request", TextFormat("Are you sure that you want to delete all non-favorited worlds? This includes %d worlds. You won't be able to recover any of them!", deletableWorldCount), PopupType::confirmation);
         megaDeleteClicked = true;
         return;
      }
      
      if (selectedButton->favorite) {
         insertPopup("Notice", TextFormat("World '%s' cannot be deleted as it is favorited. If you wish to proceed, please unfavorite it and try again.", selectedButton->text.c_str()), PopupType::info);
         return;
      }

      insertPopup("Confirmation Request", TextFormat("Are you sure that you want to delete world '%s'? You won't be able to recover it!", selectedButton->text.c_str()), PopupType::confirmation);
      deleteClicked = true;
      return;
   }

   if (megaDeleteClicked && isPopupConfirmed()) {
      int failedCount = 0;
      for (const Button &button: worldButtons) {
         if (!isWorldFavorite(button.text) && !deleteWorld(button.text)) {
            failedCount += 1;
         }
      }

      if (failedCount != 0) {
         insertPopup("Notice", TextFormat("%d worlds could not be deleted. Please check the 'data/worlds/' folder, if the files are present, check your permissions.", failedCount), PopupType::error);
      }
      loadWorldButtons();
   }
   megaDeleteClicked = false;

   if (deleteClicked && isPopupConfirmed()) {
      if (!deleteWorld(selectedButton->text)) {
         insertPopup("Notice", TextFormat("World '%s' could not be deleted. Please check the 'data/worlds/' folder, if the file is present, check your permissions.", selectedButton->text.c_str()), PopupType::error);
      }
      loadWorldButtons();
   }
   deleteClicked = false;

   if (renameButton.clicked || (!renameButton.disabled && handleKeyPressWithSound(KEY_F2))) {
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

      writeFile("data/favorites.txt", favoriteWorlds);
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
         insertPopup("Confirmation Request", TextFormat("World '%s' uses an outdated file version. The latest version is %d, whereas its version is %d. Are you sure that you want to continue? Your world might get corrupted and become unrecoverable!", selectedWorld.c_str(), getLatestVersion(), getFileVersion(selectedWorld)), PopupType::confirmation);
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
         insertPopup("Invalid World Name", TextFormat("World name must contain from %d to %d characters, but it has %d instead.", minWorldNameSize, maxWorldNameSize, worldName.text.size()), PopupType::error);
         return;
      }

      // World with the same name already exists
      for (const Button &button: worldButtons) {
         if (button.text == worldName.text) {
            insertPopup("World Exists", TextFormat("Cannot create world with the name '%s', as a world with the same name already exists.", worldName.text.c_str()), PopupType::error);
            return;
         }
      }

      generationSplash = getRandomLineFromFile("assets/config/splash.txt");
      wrapInPlace(generationSplash, font, GetScreenWidth() - mapRatioToX(0.05f, WINDOW_AREA, CUBIC_RATIO), getFontSizeScaled(40.0f));

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
         insertPopup("Invalid World Name", TextFormat("World name must contain from %d to %d characters, but it has %d instead.", minWorldNameSize, maxWorldNameSize, renameInput.text.size()), PopupType::error);
         return;
      }

      // World with the same name already exists
      for (const Button &button: worldButtons) {
         if (button.text == renameInput.text) {
            insertPopup("World Exists", TextFormat("Cannot rename world to '%s', as a world with the same name already exists.", renameInput.text.c_str()), PopupType::error);
            return;
         }
      }

      std::string newName = TextFormat("data/worlds/%s.bin", renameInput.text.c_str());
      if (std::filesystem::exists(newName) && std::filesystem::is_regular_file(newName)) {
         insertPopup("Invalid World Name", TextFormat("World with the name '%s' already exists.", renameInput.text.c_str()), PopupType::error);
         return;
      }

      std::filesystem::rename(TextFormat("data/worlds/%s.bin", selectedWorld.c_str()), newName);

      if (wasFavoriteBeforeRenaming) {
         favoriteWorlds.erase(std::remove(favoriteWorlds.begin(), favoriteWorlds.end(), renameInput.text), favoriteWorlds.end());
         favoriteWorlds.push_back(renameInput.text);
         writeFile("data/favorites.txt", favoriteWorlds);
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
   drawTextureResponsive(titleTexture, V2(0.5f, 0.3f), V2(1.0f, 0.167f), CENTER, WHITE, FULL_SOURCE, WINDOW_AREA, CUBIC_RATIO);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

// Render level selection screen

void MenuState::renderLevelSelection() {
   drawTextResponsive(font, V2(0.5f, 0.106f), "SELECT WORLD", 180.0f);
   backButton.render();
   renameButton.render();
   deleteButton.render();
   favoriteButton.render();
   playWorldButton.render();
   newButton.render();
   worldSearchBar.render();

   float scrollbarHeight = ((int)worldButtons.size() <= buttonsInWorldFrame ? 1.0f : buttonsInWorldFrame / (float)worldButtons.size());
   float scrollbarY = ((int)worldButtons.size() <= buttonsInWorldFrame ? 0.0f : (1.0f - convertRatioY(scrollbarHeight, CUBIC_RATIO, RATIO)) * (scrollIndex / float(worldButtons.size() - buttonsInWorldFrame)));
   drawTexture(scrollframeTexture, worldFrame, TOP_LEFT);
   drawTexture(scrollbarTexture, mapRatioToArea(R4(1.0f, scrollbarY, 1.222f * 0.0525f, scrollbarHeight), TOP_RIGHT, worldFrame, CUBIC_RATIO), TOP_LEFT);

   for (int i = scrollIndex; i - scrollIndex < buttonsInWorldFrame && i - scrollIndex < (int)worldButtons.size(); ++i) {
      Button &button = worldButtons[i];
      button.rect = worldFrameButtonRects[i - scrollIndex];
      button.render();

      if (button.favorite) {
         Vector2 position = {button.rect.x + (button.rect.width * button.scale) / 2.f - (button.rect.height * button.scale) / 2.f, button.rect.y};
         drawTexture(starTexture, position, mapRatioToArea(0.05f, 0.05f, WINDOW_AREA, CUBIC_RATIO));
      }
   }
}

// Render level creation screen

void MenuState::renderLevelCreation() {
   drawTextResponsive(font, V2(0.5f, 0.13f), "CREATE WORLD", 180.0f);
   backButtonCreation.render();
   createButtonCreation.render();
   worldName.render();
   shouldWorldBeFlat.render();

   Vector2 anchor = R4anchor(worldName.rect, worldName.origin, CENTER_LEFT) - mapRatioToArea(0.05f, 0.0f, WINDOW_AREA, CUBIC_RATIO);
   drawText(font, anchor, "World Name:", getFontSizeScaled(50.0f), CENTER_RIGHT);
   drawText(font, V2(anchor.x, shouldWorldBeFlat.rect.y), "Flat World:", getFontSizeScaled(50.0f), CENTER_RIGHT);
}

// Render level renaming screen

void MenuState::renderLevelRenaming() {
   drawTextResponsive(font, V2(0.5f, 0.13f), "RENAME WORLD", 180.0f);
   backButtonRenaming.render();
   renameButtonRenaming.render();
   renameInput.render();

   Vector2 anchor = R4anchor(renameInput.rect, renameInput.origin, CENTER_LEFT) - mapRatioToArea(0.05f, 0.0f, WINDOW_AREA, CUBIC_RATIO);
   Vector2 topOffset = mapRatioToArea(0.0f, 0.139f);
   drawText(font, anchor, "New World Name:", getFontSizeScaled(50.0f), CENTER_RIGHT);
   drawText(font, anchor + topOffset, "Old World Name:", getFontSizeScaled(50.0f), CENTER_RIGHT);
   drawText(font, R4anchor(renameInput.rect, renameInput.origin, CENTER) + topOffset, selectedWorld.c_str(), getFontSizeScaled(50.0f));
}

// Render level generation screen

void MenuState::renderGeneratingLevel() {
   generationProgressBar.render();
   drawTextResponsive(font, V2(0.5f, 0.5f - convertRatioY(0.0926f, RATIO, CUBIC_RATIO)), generationInfoText.c_str(), 50.0f);
   drawTextResponsive(font, V2(0.5f, 0.5f + convertRatioY(0.0926f, RATIO, CUBIC_RATIO)), generationSplash.c_str(), 40.0f);
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
   getLinesFromFileInPlace(favoriteWorlds, "data/favorites.txt");
   std::filesystem::create_directories("data/worlds/");

   worldButtons.clear();
   if (anySelected) {
      anySelected = false;
      selectedButton = nullptr;
   }

   for (const auto &file: std::filesystem::directory_iterator("data/worlds")) {
      Button button;
      button.init(font, longButtonTexture, CENTER, file.path().stem().string());
      button.favorite = isWorldFavorite(button.text);

      if (worldSearchBar.text.empty()) {
         worldButtons.push_back(button);
         continue;
      }

      std::string copyA = toLower(button.text);
      std::string copyB = toLower(worldSearchBar.text);

      if (copyA.find(copyB) != std::string::npos) {
         worldButtons.push_back(button);
      }
   }
   sortWorldButtonsByFavorites();
   scrollIndex = fmin(scrollIndex, fmax(0, (int)worldButtons.size() - buttonsInWorldFrame));
}

void MenuState::sortWorldButtonsByFavorites() {
   std::sort(worldButtons.begin(), worldButtons.end(), [](Button &a, Button &b) -> bool {
      if (a.favorite != b.favorite) {
         return a.favorite && !b.favorite;
      }
      return toLower(a.text) < toLower(b.text);
   });
}

void MenuState::resetSelection() {
   if (anySelected) {
      anySelected = false;
      selectedButton->texture = longButtonTexture;
      selectedButton = nullptr;
   }
}

void MenuState::selectButton(Button &button) {
   if (anySelected) {
      selectedButton->texture = longButtonTexture;
   }
   anySelected = true;
   selectedButton = &button;
   selectedButton->texture = longSelectedButtonTexture;
}

std::string MenuState::generateRandomWorldName() const {
   std::string adjective = getRandomLineFromFile("assets/config/adjectives.txt");
   std::string noun = getRandomLineFromFile("assets/config/nouns.txt");
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
   bool pressed = isKeyPressed(key);
   bool down = isKeyDown(key);

   if (!down) {
      repeatTimer = 0.0f;
      delayTimer = 0.0f;
      return false;
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
