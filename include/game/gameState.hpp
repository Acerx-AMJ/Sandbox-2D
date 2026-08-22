#pragma once
#include "game/state.hpp"
#include "objs/console.hpp"
#include "objs/inventory.hpp"
#include "objs/player.hpp"
#include "ui/button.hpp"

struct GameState: public State {
   enum class Phase {playing, paused, died};

   GameState(const std::string &worldName);
   ~GameState();

   // Update

   void update() override;
   void fixedUpdate() override;
   void updateResponsiveness() override;

   void updatePlaying();
   void updatePausing();
   void updateDying();

   // Physics functions

   bool handleLiquidToBlock(int x, int y, liquidid_t id);
   void updateLiquid(int x, int y, liquidid_t id);

   void updateSandPhysics(int x, int y);
   void updateGrassPhysics(int x, int y);
   void updateDirtPhysics(int x, int y);
   void updateTorchPhysics(int x, int y);

   // Other

   void render() override;
   State* change() override;

   void calculateCameraBounds();
   void pushDropTable(droptableid_t id);
   void pushPendingDroppedItems();

   // Members

   Map map;
   Player player;

   Camera2D camera;
   Rectangle cameraBounds;
   float cameraFollowSpeed = 0.416f;
   float minCameraZoom = 12.5f;
   float maxCameraZoom = 200.0f;

   Console console;
   Inventory inventory;
   Button continueButton, menuButton, pauseButton;

   std::vector<DroppedItem> droppedItems;
   std::string worldName;
   Phase phase = Phase::playing;
   Phase phaseBeforePausing = Phase::playing;

   Furniture furniturePreview;
   furnitureid_t lastFurnitureId = 0;
   blockid_t oldBlockBelowPreview = 0;
   bool flippedPreviewX = false;

   std::vector<int> liquidCounters;
   int physicsCounter = 0;
   int physicsTicks = 8;
   int grassGrowSpeedMin = 100;
   int grassGrowSpeedMax = 255;

   float deathTimer = 0.0f;
   float timeToRespawn = 10.0f;
   float maxPickupRange = 2.0f;
   float maxToolRange = 10.0f;

   // Assets

   Font font;
   Texture buttonTexture, vignetteTexture, breakingTexture, bubbleTexture, heartTexture;
   Shader grayscaleShader, waterPreviewShader;
   int waterPreviewTimeLocation = 0;
};
