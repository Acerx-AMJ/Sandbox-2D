#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

// Includes

#include "game/state.hpp"

// Menu state class

class MenuState : public State {
   enum class Phase { fadingIn, updating, fadingOut };

   Phase phase = Phase::fadingIn;
   float fadeTimer = 0.f;
   float alpha = 0.f;
   
public:
   MenuState() = default;
   ~MenuState() = default;

   static StatePtr make() {
      return std::make_unique<MenuState>();
   } 

   // Update

   void update() override;
   void updateFadingIn();
   void updateUpdating(); // Complex calculations
   void updateFadingOut();

   // Other functions

   void render() override;
   void change(States& states) override;
};

#endif
