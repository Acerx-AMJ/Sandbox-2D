#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "util/config.hpp"
#include "util/fileio.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <cmath>
#include <filesystem>

// Constructors

MenuState::MenuState()
   : backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()) {
   // Init title screen
   playButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, buttonWidth, buttonHeight};
   playButton.text = "Play";
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   optionsButton.text = "Options";
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   quitButton.text = "Quit";

   playButton.texture = optionsButton.texture = quitButton.texture = &getTexture("button");

   // Init world selection screen
   worldFrame.rectangle = {worldFramePosition.x, worldFramePosition.y, GetScreenWidth() - worldFrameSizeOffset.x, GetScreenHeight() - worldFrameSizeOffset.y};
   worldFrame.scrollHeight = worldFrame.rectangle.height;

   deleteButton.rectangle = {GetScreenWidth() / 2.f - worldButtonOffsetX, worldFrame.rectangle.y + worldFrame.rectangle.height + buttonPaddingY, buttonWidth, buttonHeight};
   deleteButton.text = "Delete World";
   deleteButton.disabled = true;
   renameButton.rectangle = {deleteButton.rectangle.x - buttonPaddingX, deleteButton.rectangle.y, buttonWidth, buttonHeight};
   renameButton.text = "Rename World";
   renameButton.disabled = true;
   backButton.rectangle = {renameButton.rectangle.x - buttonPaddingX, renameButton.rectangle.y, buttonWidth, buttonHeight};
   backButton.text = "Back";

   favoriteButton.rectangle = {GetScreenWidth() / 2.f + worldButtonOffsetX, worldFrame.rectangle.y + worldFrame.rectangle.height + buttonPaddingY, buttonWidth, buttonHeight};
   favoriteButton.text = "Favorite";
   favoriteButton.disabled = true;
   playWorldButton.rectangle = {favoriteButton.rectangle.x + buttonPaddingX, favoriteButton.rectangle.y, buttonWidth, buttonHeight};
   playWorldButton.text = "Play World";
   playWorldButton.disabled = true;
   newButton.rectangle = {playWorldButton.rectangle.x + buttonPaddingX, playWorldButton.rectangle.y, buttonWidth, buttonHeight};
   newButton.text = "New";

   backButton.texture = renameButton.texture = deleteButton.texture = favoriteButton.texture = playWorldButton.texture = newButton.texture = &getTexture("button");
   loadWorlds();

   // Init world creation screen
   backButtonCreation.rectangle = deleteButton.rectangle;
   backButtonCreation.text = "Back";
   createButton.rectangle = favoriteButton.rectangle;
   createButton.text = "Create";

   worldName.rectangle = {GetScreenWidth() / 2.f - worldNameSize.x / 2.f, GetScreenHeight() / 2.f - worldNameSize.y / 2.f, worldNameSize.x, worldNameSize.y};
   worldName.maxChars = maxWorldNameSize;
   shouldWorldBeFlat.rectangle = {GetScreenWidth() / 2.f - 35.f, worldName.rectangle.y + 200.f, 70.f, 70.f};

   backButtonCreation.texture = createButton.texture = &getTexture("button");
   resetBackground();
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::title:           updateTitle();           break;
   case Phase::levelSelection:  updateLevelSelection();  break;
   case Phase::levelCreation:   updateLevelCreation();   break;
   case Phase::generatingLevel: updateGeneratingLevel(); break;
   }
}

void MenuState::updateTitle() {
   playButton.update();
   optionsButton.update();
   quitButton.update();

   if (playButton.clicked) {
      phase = Phase::levelSelection;
   }

   if (optionsButton.clicked) {
      // Do nothing for now
   }

   if (quitButton.clicked) {
      fadingOut = true;
   }
}

void MenuState::updateLevelSelection() {
   backButton.update();
   newButton.update();
   worldFrame.update();

   float offsetY = worldFrame.getOffsetY();
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

   if (backButton.clicked) {
      phase = Phase::title;
   }

   if (newButton.clicked) {
      phase = Phase::levelCreation;
      worldName.text = getRandomWorldName();
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
      
   }

   if (renameButton.clicked) {

   }

   if (favoriteButton.clicked) {
      if (selectedButton->favorite) {
         favoriteWorlds.erase(std::remove_if(favoriteWorlds.begin(), favoriteWorlds.end(), [this](std::string &s) -> bool {
            return s == selectedButton->text;
         }));
      } else {
         favoriteWorlds.push_back(selectedButton->text);
      }

      std::string worldName = selectedButton->text;
      selectedButton->favorite = !selectedButton->favorite;
      
      saveLinesToFile("data/favorites.txt", favoriteWorlds);
      sortWorldsByFavorites();

      for (Button &button: worldButtons) {
         if (button.text == worldName) {
            selectedButton = &button;
         }
      }
   }

   if (playWorldButton.clicked) {
      selectedWorld = selectedButton->text;
      fadingOut = playing = true;
      return;
   }

   if (!anyClicked && !deleteButton.clicked && !renameButton.clicked && !favoriteButton.clicked && !playWorldButton.clicked && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (selectedButton) {
         selectedButton->texture = &getTexture("button_long");
      }
      selectedButton = nullptr;
      anySelected = false;
   }
}

void MenuState::updateLevelCreation() {
   backButtonCreation.update();
   createButton.update();
   worldName.update();
   shouldWorldBeFlat.update();

   if (backButtonCreation.clicked) {
      phase = Phase::levelSelection;
   }

   if (createButton.clicked) {
      phase = Phase::generatingLevel;
   }
}

void MenuState::updateGeneratingLevel() {
   MapGenerator generator (worldName.text, defaultMapSizeX, defaultMapSizeY, shouldWorldBeFlat.checked);
   generator.generate();

   loadWorlds();
   phase = Phase::levelSelection;
}

// Render

void MenuState::render() {
   drawBackground(foregroundTexture, backgroundTexture, parallaxBgSpeed, parallaxFgSpeed, menuSunSpeed);

   switch (phase) {
   case Phase::title:           renderTitle();           break;
   case Phase::levelSelection:  renderLevelSelection();  break;
   case Phase::levelCreation:   renderLevelCreation();   break;
   case Phase::generatingLevel: renderGeneratingLevel(); break;
   }
}

void MenuState::renderTitle() {
   drawText(getScreenCenter({0.f, titleOffsetX}), "SANDBOX 2D", 180);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

void MenuState::renderLevelSelection() {
   drawText(getScreenCenter({0.f, titleOffsetX2}), "SELECT WORLD", 180);
   backButton.render();
   renameButton.render();
   deleteButton.render();
   favoriteButton.render();
   playWorldButton.render();
   newButton.render();
   worldFrame.render();

   float offsetY = worldFrame.getOffsetY();
   for (Button &button: worldButtons) {
      if (!worldFrame.inFrame(button.normalizeRect())) {
         continue;
      }

      button.render(offsetY);
      if (button.favorite) {
         Vector2 position = {button.rectangle.x + (button.rectangle.width * button.scale) / 2.f - (button.rectangle.height * button.scale) / 2.f, button.rectangle.y - offsetY};

         drawTexture(getTexture("star"), position, {worldStarSize * button.scale, worldStarSize * button.scale});
      }
   }
}

void MenuState::renderLevelCreation() {
   drawText(getScreenCenter({0.f, titleOffsetX2}), "CREATE WORLD", 180);
   backButtonCreation.render();
   createButton.render();
   worldName.render();
   shouldWorldBeFlat.render();
   drawText({worldName.rectangle.x - 125.f, worldName.rectangle.y + worldName.rectangle.height / 2.f}, "World Name:", 50);
   drawText({worldName.rectangle.x - 125.f, shouldWorldBeFlat.rectangle.y + shouldWorldBeFlat.rectangle.height / 2.f}, "Flat World:", 50);
}

void MenuState::renderGeneratingLevel() {
   drawText(getScreenCenter(), (std::string("Generating World '") + worldName.text + "'...").c_str(), 50);
}

// Other functions

State* MenuState::change() {
   if (playing) {
      return new GameState(selectedWorld);
   }
   return nullptr;
}

void MenuState::loadWorlds() {
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
   sortWorldsByFavorites();
}

std::string MenuState::getRandomWorldName() {
   std::string adjective = getRandomLineFromFile("assets/adjectives.txt");
   std::string noun = getRandomLineFromFile("assets/nouns.txt");
   return adjective + " " + noun;
}

bool MenuState::isWorldFavorite(const std::string &name) {
   for (const std::string &world: favoriteWorlds) {
      if (world == name) {
         return true;
      }
   }
   return false;
}

void MenuState::sortWorldsByFavorites() {
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
