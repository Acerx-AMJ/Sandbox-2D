#include "game/gameState.hpp"

// Includes

#include <algorithm>
#include <cmath>
#include "PerlinNoise.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

// Constants

namespace {
   constexpr int SIZE_Y = 200;
   constexpr int SIZE_X = 1000;
}

// Constructors

GameState::GameState() {
   blocks = std::vector<std::vector<Block>>(SIZE_Y, std::vector<Block>(SIZE_X, Block{}));
   siv::PerlinNoise perlin {(siv::PerlinNoise::seed_type)rand()};
   siv::PerlinNoise sandNoise {(siv::PerlinNoise::seed_type)rand()};

   int y = SIZE_Y * 0.5f;
   
   for (int x = 0; x < SIZE_X; ++x) {
      if (blocks[y][x].tex) {
         continue;
      }
      float noise = (perlin.octave2D(x * 0.01f, y * 0.01f, 4) + 1.f) / 2.f;

      if (noise < .2f) {
         y -= random(0, 3);
      } else if (noise < .4) {
         y -= random(0, 2);
      } else if (noise < .65f) {
         y += random(-1, 1);
      } else if (noise < .85f) {
         y += random(0, 2);
      } else {
         y += random(0, 3);
      }

      if (y < SIZE_Y * .3f) {
         y++;
      } else if (y > SIZE_Y * .6f) {
         y--;
      }
      y = std::clamp(y, int(SIZE_Y * .25f), int(SIZE_Y * 65.f));
      
      blocks[y][x].tex = &ResourceManager::get().getTexture("grass"s);
      for (int yy = y + 1; yy < SIZE_Y; ++yy) {
         blocks[yy][x].tex = &ResourceManager::get().getTexture("dirt"s);
      }
   }

   for (int x = 0; x < SIZE_X; ++x) {
      for (int y = 0; y < SIZE_Y; ++y) {
         if (not blocks[y][x].tex) {
            if (y >= SIZE_Y * .45f) {
               blocks[y][x].tex = &ResourceManager::get().getTexture("water"s);
            }
            continue;
         }
         if ((sandNoise.octave2D(x * 0.01f, y * 0.01f, 4) + 1.f) / 2.f >= .75f) {
            blocks[y][x].tex = &ResourceManager::get().getTexture("sand"s);
         }
      }
   }
   camera.target = {0.f, 0.f};
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   camera.zoom = 50.f;
}

// Update functions

void GameState::update() {
   camera.offset.x += (IsKeyDown(KEY_A) - IsKeyDown(KEY_D)) * 500.f * GetFrameTime();
   camera.offset.y += (IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * 500.f * GetFrameTime();

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
      camera.offset = GetMousePosition();
      camera.target = mouseWorldPos;
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), 1.f, 5000.f);
   }

   if (IsKeyReleased(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   auto crect = getCameraBounds(camera);
   int drew = 0, total = 0;

   BeginMode2D(camera);
   auto maxY = std::min(SIZE_Y, int((crect.y + crect.height)) + 1);
   auto maxX = std::min(SIZE_X, int((crect.x + crect.width)) + 1);

   for (int y = 0; y < SIZE_Y; ++y) {
      for (int x = 0; x < SIZE_X; ++x) {
         total += (blocks[y][x].tex != nullptr);
      }
   }
   for (int y = std::max(0, int(crect.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(crect.x)); x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (not block.tex) {
            continue;
         }
         drawTextureNO(*block.tex, {(float)x, (float)y, 1.f, 1.f});
         drew++;
      }
   }
   EndMode2D();
   drawText(getScreenCenter(), ("BLOCKS RENDERED: "s + std::to_string(drew)).c_str(), 100);
   drawText({getScreenCenter().x, getScreenCenter().y + 120.f}, ("TOTAL: "s + std::to_string(total)).c_str(), 50);
   drawText({getScreenCenter().x, getScreenCenter().y - 120.f}, ("FPS: "s + std::to_string(GetFPS())).c_str(), 30);
}

void GameState::change(States& states) {
   states.push_back(GameState::make());
}
