#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <deque>
#include <memory>

// Forward declaration

class State;
using StatePtr = std::unique_ptr<State>;
using States = std::deque<StatePtr>;

// Polymorphic state

struct State {
   bool quitState = false;
   bool fadingIn = true, fadingOut = false;
   float fadeTimer = 0.f;
   float alpha = 0.f;

   // Virtual functions

   State() = default;
   virtual ~State() = default;

   virtual void update() = 0;
   virtual void render() = 0;
   virtual void change(States& states) = 0;

   // Update functions

   void updateStateLogic();
   void updateFadingIn();
   void updateFadingOut();
};

#endif
