#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

#include "game/state.hpp"
#include "objs/inventory.hpp"
#include "objs/player.hpp"
#include "ui/button.hpp"

// Game state

struct GameState: public State {
   enum class Phase {playing, paused, died};

   // Constructors

   GameState(const std::string &worldName);
   ~GameState();

   // Update

   void update() override;
   void fixedUpdate() override;

   void updatePlaying();
   void updatePausing();
   void updateDying();

   // Physics functions

   bool handleLiquidToBlock(int x, int y, LiquidType type, unsigned short blockId);
   void updateFluid(int x, int y);
   void updateWaterPhysics(int x, int y);
   void updateLavaPhysics(int x, int y);
   void updateHoneyPhysics(int x, int y);

   void updateSandPhysics(int x, int y);
   void updateGrassPhysics(int x, int y);
   void updateDirtPhysics(int x, int y);
   void updateTorchPhysics(int x, int y);

   // Other

   void render() const override;
   State* change() override;
   void calculateCameraBounds();

   // Members

   const Texture &backgroundTexture, &foregroundTexture;

   Map map;
   Player player;

   Camera2D camera;
   Rectangle cameraBounds;
   Vector2 playerSpawnPosition;

   Inventory inventory;
   Button continueButton, menuButton, pauseButton;

   std::vector<DroppedItem> droppedItems;
   std::string worldName;
   Phase phase = Phase::playing;
   Phase phaseBeforePausing = Phase::playing;

   bool canDrawPreview = false;

   float deathTimer = 0.0f;
   int physicsCounter = 0;
   int lavaCounter = 0;
   int honeyCounter = 0;
};

#endif
