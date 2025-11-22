#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

#include "game/state.hpp"
#include "objs/player.hpp"

// Game state

struct GameState: public State {
   GameState();
   ~GameState() = default;

   // Update

   void update() override;
   void updateControls();
   void updatePhysics();

   // Other functions

   void render() override;
   State* change() override;

private:
   Map map;
   Camera2D camera;
   Player player;
   float physicsTimer = 0.f;
};

#endif
