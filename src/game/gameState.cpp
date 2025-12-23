#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "util/fileio.hpp"
#include "util/input.hpp"
#include "util/math.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <algorithm>

// Constants

constexpr float cameraFollowSpeed = 0.416f;
constexpr float minCameraZoom     = 12.5f;
constexpr float maxCameraZoom     = 200.0f;

constexpr float physicsUpdateTime = 0.1f;
constexpr int lavaUpdateSpeed     = 6;
constexpr int grassGrowSpeedMin   = 100;
constexpr int grassGrowSpeedMax   = 255;

// Constructors

GameState::GameState(const std::string &worldName)
: backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()), inventory(map, player, droppedItems), worldName(worldName) {
   const Vector2 center = getScreenCenter();
   
   // Init world and camera
   resetBackground();
   loadWorldData(worldName, player, camera.zoom, map, inventory, droppedItems);

   camera.zoom = clamp(camera.zoom, minCameraZoom, maxCameraZoom);
   camera.target = player.getCenter();
   camera.offset = center;
   camera.rotation = 0.0f;
   calculateCameraBounds();

   // Init UI
   continueButton.rectangle = {center.x, center.y, buttonWidth, buttonHeight};
   continueButton.text = "Continue";
   menuButton.rectangle = {continueButton.rectangle.x, continueButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   menuButton.text = "Save & Quit";
   pauseButton.rectangle = {GetScreenWidth() - buttonWidth / 2.f + 10.0f, GetScreenHeight() - buttonHeight / 2.f, buttonWidth, buttonHeight};
   pauseButton.text = "Pause";
   continueButton.texture = menuButton.texture = &getTexture("button");
}

GameState::~GameState() {
   inventory.discardItem();
   saveWorldData(worldName, player.position.x, player.position.y, camera.zoom, map, &inventory, &droppedItems);
}

// Update

void GameState::update() {
   updatePauseScreen();
   updateControls();
   updatePhysics();
}

// Update pause screen

void GameState::updatePauseScreen() {
   pauseButton.update();
   if (pauseButton.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
      paused = !paused;
   }

   if (!paused) {
      return;
   }

   continueButton.update();
   menuButton.update();

   if (continueButton.clicked) {
      paused = false;
   }

   if (menuButton.clicked) {
      fadingOut = true;
   }
}

// Update controls

void GameState::updateControls() {
   if (!paused) {
      const float zoomFactor = IsKeyReleased(KEY_EQUAL) - IsKeyReleased(KEY_MINUS);
      if (zoomFactor != 0.f) {
         camera.zoom = clamp(std::exp(std::log(camera.zoom) + zoomFactor * 0.2f), minCameraZoom, maxCameraZoom);
      }

      player.updatePlayer(map);
      inventory.update();
   }

   camera.target = lerp(camera.target, player.getCenter(), cameraFollowSpeed);
   calculateCameraBounds();
}

// Update physics

/************************************/
// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp. Everything between these multi-comments is temporary.
static int index = 0;
static int size = 21;
static const char *blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles", "obsidian",
   "lava", "platform", "snow", "ice", "mud", "jungle_grass",
   "sapling", "cactus_seed"
};
static bool drawWall = false;
static bool canDraw = false;
static Furniture obj;
inline Furniture::Type getFurnitureType() { return (index == 19 ? Furniture::sapling : (index == 20 ? Furniture::cactus_seed : Furniture::none)); }
/************************************/

void GameState::updatePhysics() {
   if (paused) {
      return;
   }
   
   /************************************/
   // Move this to a different function later on!
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   if (IsKeyPressed(KEY_Y)) {
      index = (index + 1) % size;
   }

   if (IsKeyPressed(KEY_T)) {
      index = (index == 0 ? size - 1 : index - 1);
   }

   if (IsKeyPressed(KEY_R)) {
      drawWall =! drawWall;
   }

   if (map.isPositionValid(mousePos.x, mousePos.y)) {
      Furniture::Type ftype = getFurnitureType();
      canDraw = (drawWall || ftype != Furniture::none || !CheckCollisionRecs(player.getBounds(), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}));

      if (isMouseDownOutsideUI(MOUSE_BUTTON_LEFT)) {
         map.deleteBlock(mousePos.x, mousePos.y, drawWall);
      } else if (isMouseDownOutsideUI(MOUSE_BUTTON_RIGHT) && canDraw && !map.blocks[mousePos.y][mousePos.x].furniture) {
         if (ftype != Furniture::none) {
            Furniture::generate(mousePos.x, mousePos.y, map, ftype);
         } else {
            map.setBlock(mousePos.x, mousePos.y, blockMap[index], drawWall);
         }
      } else if (isMousePressedOutsideUI(MOUSE_BUTTON_MIDDLE) && (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].type != Block::air) {
         index = (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].id - 1;
      }
   }
   /************************************/

   // Update every frame (DT-dependant)
   if (!droppedItems.empty()) {
      const Rectangle playerBounds = player.getBounds();
      
      for (auto &droppedItem: droppedItems) {
         droppedItem.update(cameraBounds);

         if (!droppedItem.inBounds || !CheckCollisionRecs(playerBounds, droppedItem.getBounds())) {
            continue;
         }
         Item item {droppedItem.type, droppedItem.id, droppedItem.isFurniture, false, droppedItem.count};
         const int count = droppedItem.count;

         droppedItem.count = (inventory.placeItem(item) ? 0 : item.count);
         if (count != droppedItem.count) {
            playSound("pickup");
         }
      }

      droppedItems.erase(std::remove_if(droppedItems.begin(), droppedItems.end(), [](DroppedItem &i) -> bool {
         return i.flagForDeletion || i.count <= 0;
      }), droppedItems.end());
   }

   // Update in specific intervals (DT-independant)
   physicsTimer += GetFrameTime();
   if (physicsTimer >= physicsUpdateTime) {
      physicsTimer -= physicsUpdateTime;
   } else {
      return;
   }

   // Loop backwards to avoid updating most of the moving blocks twice
   for (int y = cameraBounds.height; y >= cameraBounds.y; --y) {
      for (int x = cameraBounds.width; x >= cameraBounds.x; --x) {
         Block &block = map[y][x];

         switch (block.type) {
         case Block::water: updateWaterPhysics(x, y); break;
         case Block::lava:  updateLavaPhysics(x, y);  break;
         case Block::sand:  updateSandPhysics(x, y);  break;
         case Block::grass: updateGrassPhysics(x, y); break;
         case Block::dirt:  updateDirtPhysics(x, y);  break;
         default: break;
         }
      }
   }
   
   for (Furniture &obj: map.furniture) {
      obj.update(map);
   }

   // Remove deleted furniture
   map.furniture.erase(std::remove_if(map.furniture.begin(), map.furniture.end(), [](Furniture &f) -> bool {
      return f.deleted;
   }), map.furniture.end());
}

// Block physic update functions

void GameState::updateBlockMovingAround(int &x, int y, int offsetY, const std::function<bool(const Map&, int, int)> &isPassable) {
   if (isPassable(map, x, y + 1)) {
      map.moveBlock(x, y, x, y + 1);
      return;
   }

   bool leftEmpty  = isPassable(map, x - 1, y + offsetY);
   bool rightEmpty = isPassable(map, x + 1, y + offsetY);
   if (rightEmpty && leftEmpty && chance(50)) {
      rightEmpty = false;
   }

   if (rightEmpty) {
      map.moveBlock(x, y, x + 1, y + offsetY);
   } else if (leftEmpty) {
      map.moveBlock(x, y, x - 1, y + offsetY);
      x -= 1; // Prevent the same water tile to update twice
   }
}

void GameState::updateWaterPhysics(int &x, int y) {
   updateBlockMovingAround(x, y, 0, [](const Map &map, int x, int y) -> bool {
      return map.is(x, y, Block::air); // Don't use empty, because water can go trough furniture
   });
}

void GameState::updateLavaPhysics(int &x, int y) {
   for (int yy = y - 1; yy <= y + 1 && yy >= 0 && yy < map.sizeY; ++yy) {
      for (int xx = x - 1; xx <= x + 1 && xx >= 0 && xx < map.sizeX; ++xx) {
         if (!map.isu(xx, yy, Block::water)) {
            continue;
         }

         if (map.blocks[y][x].furniture) {
            map.deleteBlock(x, y);
         } else {
            map.setBlock(x, y, "obsidian");
         }
         map.deleteBlock(xx, yy);
      }
   }

   Block &block = map[y][x];
   if (block.type != Block::lava) {
      return;
   }

   block.value += 1;
   if (block.value < lavaUpdateSpeed) {
      return;
   }

   block.value = 0;
   updateBlockMovingAround(x, y, 0, [](const Map &map, int x, int y) -> bool {
      return map.is(x, y, Block::air);
   });
}

void GameState::updateSandPhysics(int x, int y) {
   updateBlockMovingAround(x, y, 1, [](const Map &map, int x, int y) -> bool {
      return map.empty(x, y) || map.isu(x, y, Block::water);
   });
}

void GameState::updateGrassPhysics(int x, int y) {
   if (map.is(x, y - 1, Block::air) || map.is(x, y - 1, Block::water) || map.is(x, y - 1, Block::platform)) {
      return;
   }

   Block &block = map[y][x];
   if (block.value2 == 0) {
      block.value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
   }

   block.value += 1;
   if (block.value >= block.value2) {
      block.value = 0;
      block.value2 = 0;
      map.setBlock(x, y, (block.id == Block::getId("grass") ? "dirt" : "mud"));
   }
}

void GameState::updateDirtPhysics(int x, int y) {
   if (!map.is(x, y - 1, Block::air) && !map.is(x, y - 1, Block::water) && !map.is(x, y - 1, Block::platform)) {
      return;
   }

   Block &block = map[y][x];
   if (block.value2 == 0) {
      block.value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
   }

   block.value += 1;
   if (block.value >= block.value2) {
      block.value = 0;
      block.value2 = 0;
      map.setBlock(x, y, (block.id == Block::getId("dirt") ? "grass" : "jungle_grass"));
   }
}

// Render

void GameState::render() const {
   const float delta = (paused ? 0 : player.delta.x / GetFrameTime() / 60.0f); // To avoid delta time clash
   drawBackground(foregroundTexture, backgroundTexture, delta, delta, (paused ? 0.0f : 3.0f));

   BeginMode2D(camera);
   renderGame();
   EndMode2D();
   renderUI();
}

// Render game

void GameState::renderGame() const {
   map.render(cameraBounds);
   for (const DroppedItem &droppedItem : droppedItems) {
      droppedItem.render();
   }

   /************************************/
   // Scary method of rendering furniture and block preview correctly
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   if (canDraw && map.isPositionValid(mousePos.x, mousePos.y)) {
      Furniture::Type ftype = getFurnitureType();
      if (ftype != Furniture::none) {
         static Block::Type oldBelow = Block::air;
         Block::Type below = (map.isPositionValid(mousePos.x, mousePos.y + obj.sizeY) ? map.blocks[mousePos.y + obj.sizeY][mousePos.x].type : Block::air);
         
         if (ftype != obj.type || oldBelow != below) {
            obj = Furniture::get(mousePos.x, mousePos.y, map, ftype, true);
         }
         oldBelow = below;
         obj.posX = mousePos.x;
         obj.posY = mousePos.y;
         obj.preview(map);
      } else {
         drawTextureBlock(getTexture(blockMap[index]), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}, Fade((drawWall ? wallTint : (map.blocks[mousePos.y][mousePos.x].furniture ? RED : WHITE)), previewAlpha));
      }
   }
   /************************************/

   player.render();
}

// Render UI

void GameState::renderUI() const {
   inventory.render();

   if (paused) {
      continueButton.render();
      menuButton.render();
   }
   pauseButton.render();
}

// Change states

State* GameState::change() {
   return new MenuState();
}

// Helper functions

void GameState::calculateCameraBounds() {
   cameraBounds = getCameraBounds(camera);

   cameraBounds.x = max(0, int(cameraBounds.x));
   cameraBounds.y = max(0, int(cameraBounds.y));
   cameraBounds.width = min(map.sizeX - 1, int(cameraBounds.x + cameraBounds.width) + 1);
   cameraBounds.height = min(map.sizeY - 1, int(cameraBounds.y + cameraBounds.height) + 1);
}
