#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

// Includes

#include <deque>
#include <memory>

// Forward declaration

class State;
using StatePtr = std::unique_ptr<State>;
using States = std::deque<StatePtr>;

// Polymorphic state class

class State {
public:
   bool quitState = false;

   State() = default;
   virtual ~State() = default;

   virtual void update() = 0;
   virtual void render() = 0;
   virtual void change(States& states) = 0;
};

#endif
