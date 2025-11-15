#include "game/gameState.hpp"

// Includes

#include <algorithm>
#include <cmath>
#include "util/math.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

// Constants

constexpr int mapSizeX = 1250;
constexpr int mapSizeY = 500;

constexpr float speed = 20.f;
constexpr float jumpSpeed = -20.f;
constexpr float gravity = 15.f;
constexpr float maxGravity = 45.f;
constexpr float friction = 16.f;
constexpr float acceleration = 16.f;

constexpr float jumpHoldTime = .2f;

// Constructors

GameState::GameState() {
   generateMap(blocks, mapSizeX, mapSizeY);
   playerPos = {mapSizeX / 2.f, 0.f};
   playerVel = {0, 0};

   camera.target = playerPos;
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   camera.zoom = 50.f;
}

// Update functions

void GameState::update() {

   // Update player

   playerVel.y = std::min(maxGravity * GetFrameTime(), playerVel.y + gravity * GetFrameTime());
   auto dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (dir != 0) {
      auto speedX = (onGround ? speed : speed * 1.4f);
      playerVel.x = lerp(playerVel.x, dir * speed * GetFrameTime(), acceleration * GetFrameTime());
   } else {
      playerVel.x = lerp(playerVel.x, 0.0, friction * GetFrameTime());
   }

   if (IsKeyDown(KEY_SPACE) and canHoldJump) {
      if (playerVel.y > 0.f) {
         playerVel.y = 0.f;
      }
      playerVel.y += jumpSpeed * GetFrameTime();

      holdJumpTimer += GetFrameTime();
      if (holdJumpTimer >= jumpHoldTime) {
         canHoldJump = false;
      }
   }

   if (onGround) {
      canHoldJump = true;
      holdJumpTimer = 0.f;
   } else if (not IsKeyDown(KEY_SPACE)) {
      canHoldJump = false;
   }

   Rectangle bounds {playerPos.x + playerVel.x, playerPos.y + playerVel.y, 2.f, 3.f};
   bool collisionX = false, collisionY = false;

   auto maxY = std::min(mapSizeY, int((bounds.y + bounds.height)) + 1);
   auto maxX = std::min(mapSizeX, int((bounds.x + bounds.width)) + 1);

   for (int y = std::max(0, int(bounds.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(bounds.x)); x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (block.type == Block::Type::air or block.type == Block::Type::water) {
            continue;
         }

         if (not CheckCollisionRecs(bounds, {(float)x, (float)y, 1, 1})) {
            continue;
         }
         auto overlapX = std::min((playerPos.x + 2.f) - x, (x + 1) - playerPos.x);
         auto overlapY = std::min((playerPos.y + 3.f) - y, (y + 1) - playerPos.y);

         if (overlapX == 0.f and overlapY == 0.f) {
            continue;
         }

         if (overlapX < overlapY) {
            if (x > playerPos.x) {
               playerPos.x = x - 2.f;
            } else {
               playerPos.x = x + 1.f;
            }
            collisionX = true;
         } else {
            if (y > playerPos.y) {
               playerPos.y = y - 3.f;
               onGround = true;
            } else {
               playerPos.y = y + 1.f;
            }
            collisionY = true;
         }

         if (collisionX and collisionY) {
            goto breakLoop;
         }
      }
   }
breakLoop:

   if (not collisionX) {
      playerPos.x += playerVel.x;
   }

   if (not collisionY) {
      onGround = false;
      playerPos.y += playerVel.y;
   }

   // Update camera/game   

   camera.target = {playerPos.x + 2.f / 2.f, playerPos.y + 3.f / 2.f};

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
   auto crect = getCameraBounds(camera);
   int drew = 0, onScreen = 0;

   BeginMode2D(camera);
   auto maxY = std::min(mapSizeY, int((crect.y + crect.height)) + 1);
   auto maxX = std::min(mapSizeX, int((crect.x + crect.width)) + 1);

   for (int y = std::max(0, int(crect.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(crect.x)); x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (block.type == Block::Type::air) {
            continue;
         }

         if (camera.zoom <= 12.5f) {
            int ox = x;
            while (x < maxX and blocks[y][x].id == block.id) {
               ++x;
            }
            DrawRectangle(ox, y, x - ox, 1, getBlockColor(block.id));
            onScreen += x - ox;
            --x;
         } else {
            drawTextureNO(*block.tex, {(float)x, (float)y, 1.f, 1.f});
            onScreen++;
         }
         drew++;
      }
   }

   DrawRectanglePro({playerPos.x, playerPos.y, 2.f, 3.f}, {0.f, 0.f}, 0.f, RED);
   EndMode2D();
   drawText(getScreenCenter(), ("RECTANGLES RENDERED: "s + std::to_string(drew)).c_str(), 80);
   drawText({getScreenCenter().x, getScreenCenter().y + 60.f}, ("ON SCREEN: "s + std::to_string(onScreen)).c_str(), 50);
   drawText({getScreenCenter().x, getScreenCenter().y - 120.f}, ("FPS: "s + std::to_string(GetFPS())).c_str(), 30);
}

void GameState::change(States& states) {
   states.push_back(GameState::make());
}
