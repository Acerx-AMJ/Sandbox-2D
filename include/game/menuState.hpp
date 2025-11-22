#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

#include "game/state.hpp"
#include "ui/button.hpp"

// Menu state

struct MenuState: public State {
   MenuState();
   ~MenuState() = default;

   // Update

   void update() override;

   // Other functions

   void render() override;
   State* change() override;

private:
   Button playButton, optionsButton, quitButton;
   bool playing = false;
};

#endif
