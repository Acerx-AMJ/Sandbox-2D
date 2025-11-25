#include <filesystem>
#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "util/fileio.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constants

constexpr int defaultMapSizeX = 2000;
constexpr int defaultMapSizeY = 750;

// Constructors

MenuState::MenuState() {
   // Init title screen
   playButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, 210.f, 70.f};
   playButton.text = "Play";
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + 90.f, 210.f, 70.f};
   optionsButton.text = "Options";
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + 90.f, 210.f, 70.f};
   quitButton.text = "Quit";

   // Init world selection screen
   worldFrame.rectangle = {300.f, 200.f, GetScreenWidth() - 600.f, GetScreenHeight() - 400.f};
   worldFrame.scrollHeight = worldFrame.rectangle.height;
   backButton.rectangle = {GetScreenWidth() / 2.f - 120.f, worldFrame.rectangle.y + worldFrame.rectangle.height + 90.f, 210.f, 70.f};
   backButton.text = "Back";
   newButton.rectangle = {GetScreenWidth() / 2.f + 120.f, worldFrame.rectangle.y + worldFrame.rectangle.height + 90.f, 210.f, 70.f};
   newButton.text = "New";

   loadWorlds();

   // Init world creation screen
   createButton.rectangle = newButton.rectangle;
   createButton.text = "Create";
   worldName.rectangle = {GetScreenWidth() / 2.f - 210.f, GetScreenHeight() / 2.f - 70.f, 420.f, 140.f};
   worldName.maxChars = 24;
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::title:          updateTitle();          break;
   case Phase::levelSelection: updateLevelSelection(); break;
   case Phase::levelCreation:  updateLevelCreation();  break;
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
   for (auto& button: worldButtons) {
      if (worldFrame.inFrame(button.normalizeRect())) {
         button.update(offsetY);

         if (button.clicked) {
            selectedWorld = button.text;
            fadingOut = playing = true;
         }
      }
   }

   if (backButton.clicked) {
      phase = Phase::title;
   }

   if (newButton.clicked) {
      phase = Phase::levelCreation;
      worldName.text = getRandomWorldName();
   }
}

void MenuState::updateLevelCreation() {
   backButton.update();
   createButton.update();
   worldName.update();

   if (backButton.clicked) {
      phase = Phase::levelSelection;
   }

   if (createButton.clicked) {
      generateMap(worldName.text, defaultMapSizeX, defaultMapSizeY);
      loadWorlds();
      phase = Phase::levelSelection;
   }
}

// Render

void MenuState::render() {
   switch (phase) {
   case Phase::title:          renderTitle();          break;
   case Phase::levelSelection: renderLevelSelection(); break;
   case Phase::levelCreation:  renderLevelCreation();  break;
   }
}

void MenuState::renderTitle() {
   drawText(getScreenCenter(0.f, -200.f), "SANDBOX 2D", 180);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

void MenuState::renderLevelSelection() {
   drawText(getScreenCenter(0.f, -400.f), "SELECT WORLD", 180);
   backButton.render();
   newButton.render();
   worldFrame.render();

   float offsetY = worldFrame.getOffsetY();
   for (auto& button: worldButtons) {
      if (worldFrame.inFrame(button.normalizeRect())) {
         button.render(offsetY);
      }
   }
}

void MenuState::renderLevelCreation() {
   drawText(getScreenCenter(0.f, -400.f), "CREATE WORLD", 180);
   backButton.render();
   createButton.render();
   worldName.render();
   drawText({worldName.rectangle.x - 125.f, worldName.rectangle.y + worldName.rectangle.height / 2.f}, "World Name:", 50);
}

// Other functions

State* MenuState::change() {
   if (playing) {
      return new GameState(selectedWorld);
   }
   return nullptr;
}

void MenuState::loadWorlds() {
   worldButtons.clear();
   std::filesystem::create_directories("data/worlds/");
   for (const auto& file: std::filesystem::directory_iterator("data/worlds")) {
      Button button;
      button.rectangle = {360.f, 210.f + 110.f * worldButtons.size(), worldFrame.rectangle.width - 120.f, 100.f};
      button.rectangle.x += button.rectangle.width / 2.f;
      button.rectangle.y += button.rectangle.height / 2.f;
      button.text = file.path().stem().string();
      worldButtons.push_back(button);
      worldFrame.scrollHeight = button.rectangle.y + button.rectangle.height / 2.f;
   }
}

std::string MenuState::getRandomWorldName() {
   auto adjective = getRandomLineFromFile("assets/adjectives.txt");
   auto noun = getRandomLineFromFile("assets/nouns.txt");
   return adjective + " " + noun;
}
