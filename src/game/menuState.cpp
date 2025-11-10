#include "game/menuState.hpp"

// Includes

#include "game/gameState.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

// Constructors

MenuState::MenuState() {
   playButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, 210.f, 70.f};
   playButton.text = "Play"s;
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + 90.f, 210.f, 70.f};
   optionsButton.text = "Options"s;
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + 90.f, 210.f, 70.f};
   quitButton.text = "Quit"s;
}

// Update

void MenuState::update() {
   playButton.update();
   optionsButton.update();
   quitButton.update();

   if (playButton.clicked) {
      fadingOut = playing = true;
   }

   if (optionsButton.clicked) {

   }

   if (quitButton.clicked) {
      fadingOut = true;
   }
}

// Other functions

void MenuState::render() {
   drawText(getScreenCenter(0.f, -200.f), "TERRARIA", 180);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

void MenuState::change(States& states) {
   if (playing) {
      states.push_back(GameState::make());
   }
}
