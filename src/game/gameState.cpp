#include <algorithm>
#include <cmath>
#include "game/gameState.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

// Constants

constexpr int mapSizeX = 1250;
constexpr int mapSizeY = 500;

// Constructors

GameState::GameState() {
   generateMap(blocks, mapSizeX, mapSizeY);
   player.init({mapSizeX / 2.f, 0.f});

   camera.target = player.pos;
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
   camera.target = player.getCenter();

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), 1.f, 5000.f);
   }

   if (IsKeyReleased(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

void GameState::updatePhysics() {
   auto mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   // Temporary way to switch, delete and place blocks
   static int index = 0;
   static int size = 7;
   static const char* blockMap[] {
      "stone", "clay", "dirt", "grass", "sand", "sandstone", "water"
   };

   if (IsKeyPressed(KEY_E)) {
      index = (index + 1) % size;
   }

   if (isPositionValid(blocks, mousePos.x, mousePos.y)) {

      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
         deleteBlock(blocks[mousePos.y][mousePos.x]);
      } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
         setBlock(blocks[mousePos.y][mousePos.x], blockMap[index]);
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
               moveBlock(block, blocks[y][x + (chance(50) ? 1.f : -1.f)]);
            } else if (leftIs(blocks, x, y)) {
               moveBlock(block, blocks[y][x - 1]);
            } else if (rightIs(blocks, x, y)) {
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
}

void GameState::change(States& states) {
   states.push_back(GameState::make());
}
