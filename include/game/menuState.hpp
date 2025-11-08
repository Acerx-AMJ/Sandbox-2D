#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

// Includes

#include "game/state.hpp"

// Menu state class

class MenuState : public State {
public:
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
};

#endif
