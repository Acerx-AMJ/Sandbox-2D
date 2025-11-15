#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

// Includes

#include "game/state.hpp"
#include "objs/block.hpp"
#include <vector>

// Game state

struct GameState: public State {
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

private:   
   std::vector<std::vector<Block>> blocks;
   Camera2D camera;
   Vector2 playerPos, playerVel;
   bool onGround = false, canHoldJump = true;
   float holdJumpTimer = 0.f;
};

#endif
