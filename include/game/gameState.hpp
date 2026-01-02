#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

#include "game/state.hpp"
#include "objs/inventory.hpp"
#include "objs/player.hpp"
#include "ui/button.hpp"

// Game state

struct GameState: public State {
   // Constructors

   GameState(const std::string &worldName);
   ~GameState();

   // Update

   void update() override;
   void fixedUpdate() override;

   void updatePauseScreen();
   void updateControls();
   void updatePhysics();

   void updateFluid(int x, int y);
   void updateWaterPhysics(int x, int y);
   void updateLavaPhysics(int x, int y);
   void updateSandPhysics(int x, int y);
   void updateGrassPhysics(int x, int y);
   void updateDirtPhysics(int x, int y);
   void updateTorchPhysics(int x, int y);

   // Render

   void render() const override;
   void renderGame() const;
   void renderUI() const;

   // Change states

   State* change() override;

   // Helpler functions

   void calculateCameraBounds();

   // Members

   const Texture &backgroundTexture, &foregroundTexture;

   Map map;
   Player player;

   Camera2D camera;
   Rectangle cameraBounds;

   Inventory inventory;
   Button continueButton, menuButton, pauseButton;

   std::vector<DroppedItem> droppedItems;
   std::string worldName;

   bool paused = false;
   int physicsCounter = 0;
   int lavaCounter = 0;
};

#endif
