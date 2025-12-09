#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "util/fileio.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <cmath>
#include <filesystem>

// Constants

constexpr int defaultMapSizeX = 2000;
constexpr int defaultMapSizeY = 750;

// Constructors

MenuState::MenuState()
   : backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()) {
   // Init title screen
   playButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, 210.f, 70.f};
   playButton.text = "Play";
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + 90.f, 210.f, 70.f};
   optionsButton.text = "Options";
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + 90.f, 210.f, 70.f};
   quitButton.text = "Quit";

   // Init world selection screen
   worldFrame.rectangle = {280.f, 200.f, GetScreenWidth() - 600.f, GetScreenHeight() - 360.f};
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
   worldName.maxChars = 48;
   shouldWorldBeFlat.rectangle = {GetScreenWidth() / 2.f - 35.f, worldName.rectangle.y + 200.f, 70.f, 70.f};

   playButton.texture = optionsButton.texture = quitButton.texture = backButton.texture = newButton.texture = createButton.texture = &getTexture("button");
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
   for (Button &button: worldButtons) {
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
   shouldWorldBeFlat.update();

   if (backButton.clicked) {
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
   // Render the parallax background
   drawTextureNoOrigin(getTexture("sky"), {0, 0}, getScreenSize());
   drawParallaxTexture(backgroundTexture, scrollingBg, 75.f, true);
   drawParallaxTexture(foregroundTexture, scrollingFg, 100.f, false);

   // Render everything else
   switch (phase) {
   case Phase::title:           renderTitle();           break;
   case Phase::levelSelection:  renderLevelSelection();  break;
   case Phase::levelCreation:   renderLevelCreation();   break;
   case Phase::generatingLevel: renderGeneratingLevel(); break;
   }
}

void MenuState::renderTitle() {
   drawText(getScreenCenter({0.f, -200.f}), "SANDBOX 2D", 180);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

void MenuState::renderLevelSelection() {
   drawText(getScreenCenter({0.f, -400.f}), "SELECT WORLD", 180);
   backButton.render();
   newButton.render();
   worldFrame.render();

   float offsetY = worldFrame.getOffsetY();
   for (Button &button: worldButtons) {
      if (worldFrame.inFrame(button.normalizeRect())) {
         button.render(offsetY);
      }
   }
}

void MenuState::renderLevelCreation() {
   drawText(getScreenCenter({0.f, -400.f}), "CREATE WORLD", 180);
   backButton.render();
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
   worldButtons.clear();
   std::filesystem::create_directories("data/worlds/");
   float offsetX = 56.666f;

   for (const auto &file: std::filesystem::directory_iterator("data/worlds")) {
      Button button;
      button.rectangle = {360.f - offsetX / 2.f, 210.f + 110.f * worldButtons.size(), worldFrame.rectangle.width - 120.f - offsetX / 2.f, 100.f};
      button.rectangle.x += button.rectangle.width / 2.f;
      button.rectangle.y += button.rectangle.height / 2.f;
      button.text = file.path().stem().string();
      button.texture = &getTexture("button_long");
      worldButtons.push_back(button);
      worldFrame.scrollHeight = std::max(worldFrame.rectangle.height, button.rectangle.y + button.rectangle.height / 2.f);
   }
}

std::string MenuState::getRandomWorldName() {
   std::string adjective = getRandomLineFromFile("assets/adjectives.txt");
   std::string noun = getRandomLineFromFile("assets/nouns.txt");
   return adjective + " " + noun;
}
