#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constructors

MenuState::MenuState() {
   playButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, 210.f, 70.f};
   playButton.text = "Play";
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + 90.f, 210.f, 70.f};
   optionsButton.text = "Options";
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + 90.f, 210.f, 70.f};
   quitButton.text = "Quit";
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
   drawText(getScreenCenter(0.f, -200.f), "SANDBOX 2D", 180);
   playButton.render();
   optionsButton.render();
   quitButton.render();
}

void MenuState::change(States& states) {
   if (playing) {
      states.push_back(GameState::make());
   }
}
