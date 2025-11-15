#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

#include "game/state.hpp"
#include "util/button.hpp"

// Menu state

struct MenuState: public State {
   MenuState();
   ~MenuState() = default;

   static StatePtr make() {
      return std::make_unique<MenuState>();
   } 

   // Update

   void update() override;

   // Other functions

   void render() override;
   void change(States& states) override;

private:
   Button playButton, optionsButton, quitButton;
   bool playing = false;
};

#endif
