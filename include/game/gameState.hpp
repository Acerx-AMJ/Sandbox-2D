#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

#include "game/state.hpp"
#include "objs/inventory.hpp"
#include "objs/player.hpp"
#include "ui/button.hpp"

// Game state

struct GameState: public State {
   GameState(const std::string &worldName);
   ~GameState();

   // Update

   void update() override;
   void updatePauseScreen();
   void updateControls();
   void updatePhysics();

   // Other functions

   void render() const override;
   State* change() override;

   void calculateCameraBounds();

private:
   Map map;
   Camera2D camera;
   Player player;
   Inventory inventory;

   Rectangle cameraBounds;
   std::vector<DroppedItem> droppedItems;

   Texture &backgroundTexture, &foregroundTexture;
   std::string worldName;
   float physicsTimer = 0.0f, updateTimer = 0.0f;

   Button continueButton, menuButton, pauseButton;
   bool paused = false;
};

#endif
