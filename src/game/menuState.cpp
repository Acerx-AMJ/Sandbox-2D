#include <fstream>
#include "json.hpp"
#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

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
   worldFrame.scrollHeight = worldFrame.rectangle.height * 2.f;
   backButton.rectangle = {GetScreenWidth() / 2.f - 120.f, worldFrame.rectangle.y + worldFrame.rectangle.height + 90.f, 210.f, 70.f};
   backButton.text = "Back";
   newButton.rectangle = {GetScreenWidth() / 2.f + 120.f, worldFrame.rectangle.y + worldFrame.rectangle.height + 90.f, 210.f, 70.f};
   newButton.text = "New";

   loadWorlds();
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::title:          updateTitle();          break;
   case Phase::levelSelection: updateLevelSelection(); break;
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
      button.update(offsetY);
   }

   if (backButton.clicked) {
      phase = Phase::title;
   }

   if (newButton.clicked) {
      fadingOut = playing = true;
   }
}

// Render

void MenuState::render() {
   switch (phase) {
   case Phase::title:          renderTitle();          break;
   case Phase::levelSelection: renderLevelSelection(); break;
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
   DrawRectangle(GetMouseX(), GetMouseY(), 50, 50, RED);
}

// Other functions

State* MenuState::change() {
   if (playing) {
      return new GameState();
   }
   return nullptr;
}

void MenuState::loadWorlds() {
   std::fstream file ("data/worlds.json");
   if (not file.is_open()) {
      warn("File 'data/worlds.json' could not be opened.", 0);
      return;
   }

   nlohmann::json data;
   file >> data;
   file.close();

   if (not data.contains("worlds") or not data["worlds"].is_array()) {
      warn("Invalid 'data/worlds.json' format: expected entry 'worlds' to be an 'array', but it is '{}' instead.", data["worlds"].type_name());
      return;
   }

   worldButtons.reserve(data["worlds"].size());
   for (int i = 0; i < data["worlds"].size(); ++i) {
      auto& entry = data["worlds"][i];

      // Laziness at its finest
      if (not entry.is_object() or not entry.contains("name") or not entry["name"].is_string() or not entry.contains("map") or not entry["map"].is_array()) {
         warn("Invalid 'data/worlds.json' format: '{}' is not a valid world structure.", entry.dump());
         continue;
      }

      Button button;
      button.rectangle = {360.f, 210.f + 110.f * worldButtons.size(), worldFrame.rectangle.width - 120.f, 100.f};
      button.rectangle.x += button.rectangle.width / 2.f;
      button.rectangle.y += button.rectangle.height / 2.f;
      button.text = entry["name"];
      button.index = i;
      worldButtons.push_back(button);
   }
}
