#include <algorithm>
#include <cmath>
#include "game/gameState.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

// Constants

constexpr int mapSizeX = 1250;
constexpr int mapSizeY = 500;

constexpr float cameraFollowSpeed = 17.5f;
constexpr float maxCameraZoom = 1.f;
constexpr float minCameraZoom = 100.f;

// Constructors

GameState::GameState() {
   generateMap(blocks, mapSizeX, mapSizeY);
   player.init({mapSizeX / 2.f, 0.f});

   camera.target = player.getCenter();
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   camera.zoom = 50.f;
}

// Update functions

void GameState::update() {
   updateControls();
   updatePhysics();
}

void GameState::updateControls() {
   player.updatePlayer(blocks);
   camera.target = lerp(camera.target, player.getCenter(), 25.f * GetFrameTime());

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), maxCameraZoom, minCameraZoom);
   }

   if (IsKeyReleased(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp.
static int index = 0;
static int size = 12;
static const char* blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles"
};

void GameState::updatePhysics() {
   auto mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   if (IsKeyPressed(KEY_E)) {
      index = (index + 1) % size;
   }

   if (IsKeyPressed(KEY_Q)) {
      index = (index == 0 ? size - 1 : index - 1);
   }

   if (isPositionValid(blocks, mousePos.x, mousePos.y)) {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
         deleteBlock(blocks[mousePos.y][mousePos.x]);
      } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
         setBlock(blocks[mousePos.y][mousePos.x], blockMap[index]);
      } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) and blocks[mousePos.y][mousePos.x].type != Block::Type::air) {
         index = blocks[mousePos.y][mousePos.x].id - 1;
      }
   }

   physicsTimer += GetFrameTime();
   if (physicsTimer >= .1f) {
      physicsTimer -= .1f;
   } else {
      return;
   }

   
   for (int y = mapSizeY - 1; y >= 0; --y) {
      for (int x = mapSizeX - 1; x >= 0; --x) {
         auto& block = blocks[y][x];

         if (block.type == Block::Type::water) {
            if (bottomIs(blocks, x, y)) {
               moveBlock(block, blocks[y + 1][x]);
            } else if (leftIs(blocks, x, y) and rightIs(blocks, x, y)) {
               if (chance(50)) {
                  goto moveWaterLeft;
               } else {
                  goto moveWaterRight;
               }
            } else if (leftIs(blocks, x, y)) {
            moveWaterLeft:
               moveBlock(block, blocks[y][x - 1]);
               --x; // Prevent the water tile from updating twice
            } else if (rightIs(blocks, x, y)) {
            moveWaterRight:
               moveBlock(block, blocks[y][x + 1]);
            }
         }

         if (block.type == Block::Type::sand and (bottomIs(blocks, x, y) or bottomIs(blocks, x, y, Block::Type::water))) {
            moveBlock(block, blocks[y + 1][x]);
         }

         if (block.type == Block::Type::dirt and (topIs(blocks, x, y) or topIs(blocks, x, y, Block::Type::water)) and chance(1)) {
            setBlock(block, "grass");
         }

         if (block.type == Block::Type::grass and not topIs(blocks, x, y) and not topIs(blocks, x, y, Block::Type::water) and chance(1)) {
            setBlock(block, "dirt");
         }
      }
   }
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   BeginMode2D(camera);

   auto crect = getCameraBounds(camera);
   auto maxY = std::min(mapSizeY, int((crect.y + crect.height)) + 1);
   auto maxX = std::min(mapSizeX, int((crect.x + crect.width)) + 1);

   for (int y = std::max(0, int(crect.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(crect.x)); x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (block.type == Block::Type::air) {
            continue;
         }

         int ox = x;
         while (x < maxX and blocks[y][x].id == block.id) { ++x; }

         if (camera.zoom <= 12.5f) {
            DrawRectangle(ox, y, x - ox, 1, getBlockColor(block.id));
         } else {
            drawTextureBlock(*block.tex, {(float)ox, (float)y, float(x - ox), 1.f});
         }
         --x;
      }
   }
   player.render();
   EndMode2D();

   drawTexture(ResourceManager::get().getTexture(blockMap[index]), {GetScreenWidth() - 75.f, GetScreenHeight() - 75.f}, {50.f, 50.f});
}

void GameState::change(States& states) {
   states.push_back(GameState::make());
}
