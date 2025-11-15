#include "game/gameState.hpp"

// Includes

#include <algorithm>
#include <cmath>
#include "util/position.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

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
