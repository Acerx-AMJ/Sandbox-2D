#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

// Includes

#include "game/state.hpp"

// Game state class

class GameState: public State {
public:
   GameState();
   ~GameState() = default;

   static StatePtr make() {
      return std::make_unique<GameState>();
   }

   // Update

   void update() override;

   // Other functions

   void render() override;
   void change(States& states) override;
};

#endif
