#include "game/state.hpp"
#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "mngr/sound.hpp"
#include "util/math.hpp"
#include "util/position.hpp"
#include <raymath.h>

// Player's keybinds shouldn't overlap with any other keybinds in GameState. It
// should be non-blocking, so our input manager is not used here.

// Constants

constexpr Vector2 playerSize    = {1.8f, 2.7f};
constexpr float playerFrameSize = 16;

constexpr float playerSpeed   = 0.364f;
constexpr float airMultiplier = 0.600f;
constexpr float jumpSpeed     = 0.950f;
constexpr float gravity       = 0.028f;
constexpr float maxGravity    = 1.333f;
constexpr float acceleration  = 0.083f;
constexpr float deceleration  = 0.167f;

constexpr float coyoteTime = 0.1f;
constexpr float foxTime    = 0.1f;
constexpr float jumpTime   = 0.25f;

// Constructors

void Player::init() {
   delta = velocity = {0, 0};
   previousPosition = position;
}

// Update

void Player::updatePlayer(Map &map) {
   updateMovement();
   updateCollisions(map);
   updateAnimation();

   delta = {position.x - previousPosition.x, position.y - previousPosition.y};
   previousPosition = position;
}

void Player::updateMovement() {
   // Handle gravity
   if (!onGround) {
      velocity.y += gravity * waterMultiplier;
      if (velocity.y >= maxGravity) {
         velocity.y = maxGravity * waterMultiplier;
      }
   } else {
      velocity.y = 0.001f; // Needed
   }

   // Handle movement
   int directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (directionX != 0) {
      float speedX = (onGround ? playerSpeed : playerSpeed * airMultiplier);
      velocity.x = lerp(velocity.x, directionX * speedX, acceleration * iceMultiplier);
   } else {
      velocity.x = lerp(velocity.x, 0.f, deceleration * iceMultiplier);
   }

   // Handle jumping
   if (!onGround && IsKeyDown(KEY_SPACE)) {
      foxTimer = foxTime;
   } else {
      foxTimer -= fixedUpdateDT;
   }

   if (onGround) {
      coyoteTimer = coyoteTime;
   } else {
      coyoteTimer -= fixedUpdateDT;
   }

   jumpTimer -= fixedUpdateDT;
   if (((IsKeyDown(KEY_SPACE) && coyoteTimer > 0) || (onGround && foxTimer > 0)) && jumpTimer <= 0) {
      playSound("jump");
      velocity.y = -jumpSpeed;
      coyoteTimer = 0.f;
      jumpTimer = jumpTime;
   }

   // Do everything else
   velocity.x *= waterMultiplier;

   if (directionX != 0 ) {
      flipX = (directionX == 1);
   }
}

// Update collisions

void Player::updateCollisions(Map &map) {
   position.y += velocity.y;

   bool collisionY = false, canGoUpSlopes = true;
   int waterTileCount = 0, lavaTileCount = 0, iceTileCount = 0;

   if (position.y < 0) {
      velocity.y = max(0.f, velocity.y);
      position.y = 0;
      canGoUpSlopes = onGround = false;
      collisionY = true;
   } else if (position.y > map.sizeY - playerSize.y) {
      position.y = map.sizeY - playerSize.y;
      onGround = collisionY = true;
   }

   int maxX = min(map.sizeX, int(position.x + playerSize.x) + 1);
   int maxY = min(map.sizeY, int(position.y + playerSize.y) + 1);

   for (int y = max(0, (int)position.y); y < maxY; ++y) {
      for (int x = max(0, (int)position.x); x < maxX; ++x) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (IsKeyDown(KEY_S) && map.isu(x, y, Block::platform)) || map.isu(x, y, Block::torch)) {
            // Only check water and lava tile count in the first iteration
            waterTileCount += (map.isu(x, y, Block::water) && map[y][x].value2 > playerThreshold);
            lavaTileCount += (map.isu(x, y, Block::lava) && map[y][x].value2 > playerThreshold);
            continue;
         }

         if (!CheckCollisionRecs({position.x, position.y, playerSize.x, playerSize.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (previousPosition.y >= y + 1.f && !map.isu(x, y, Block::platform)) {
            velocity.y = max(0.f, velocity.y);
            position.y = y + 1.f;
            collisionY = true;
            onGround = false;
         }

         if (previousPosition.y + playerSize.y <= y) {
            position.y = y - playerSize.y;
            onGround = true;
            collisionY = true;
            iceTileCount += map.isu(x, y, Block::ice);
         }
      }
   }

   if (!torsoCollision && feetCollision && !IsKeyDown(KEY_S)) {
      position.y = feetCollisionY - playerSize.y;
   }

   position.x += velocity.x;

   Rectangle torso {position.x, position.y - 1.f, playerSize.x, playerSize.y};
   Rectangle feet {position.x, position.y + (playerSize.y - 1.f), playerSize.x, 1.f};
   torsoCollision = feetCollision = false;
   feetCollisionY = 0;

   position.x = clamp(position.x, 0.f, map.sizeX - playerSize.x);
   maxX = min(map.sizeX, int(position.x + playerSize.x) + 1);
   maxY = min(map.sizeY, int(position.y + playerSize.y) + 1);

   for (int y = max(0, (int)position.y - 1); y < maxY; ++y) {
      for (int x = max(0, (int)position.x); x < maxX; ++x) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (map.isu(x, y, Block::platform) && !IsKeyDown(KEY_W)) || map.isu(x, y, Block::torch)) {
            continue;
         }

         if (canGoUpSlopes && !feetCollision && CheckCollisionRecs(feet, {(float)x, (float)y, 1.f, 1.f})) {
            feetCollision = true;
            feetCollisionY = y;
         }

         if (map.isu(x, y, Block::platform)) {
            continue;
         }

         if (!torsoCollision && (CheckCollisionRecs(torso, {(float)x, (float)y, 1.f, 1.f}) || position.y <= 0.f)) {
            torsoCollision = true;
         }

         if (!CheckCollisionRecs({position.x, position.y, playerSize.x, playerSize.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (previousPosition.x >= x + 1.f) {
            position.x = x + 1.f;
         }

         if (previousPosition.x + playerSize.x <= x) {
            position.x = x - playerSize.x;
         }
      }
   }

   position.x = clamp(position.x, 0.f, map.sizeX - playerSize.x);
   position.y = clamp(position.y, 0.f, map.sizeY - playerSize.y);

   if (lavaTileCount > 0) {
      waterMultiplier = .6f;
   } else if (waterTileCount > 0) {
      waterMultiplier = .85f;
   } else {
      waterMultiplier = 1.f;
   }

   if (!collisionY) {
      onGround = false;
   }
   if (onGround) {
      iceMultiplier = (iceTileCount > 0 ? .2f : 1.f);
   }
}

void Player::updateAnimation() {
   if (!onGround) {
      fallTimer += fixedUpdateDT;
      if (fallTimer >= .05f) {
         frameX = 1;
      }
      return;
   }

   fallTimer = 0.f;
   if (position.x == previousPosition.x) {
      frameX = 0;
      return;
   }

   walkTimer += clamp(abs(velocity.x) / playerSpeed, 0.1f, 1.5f) * fixedUpdateDT;
   if (walkTimer >= .04f) {
      int lastFrameX = frameX;
      frameX = (frameX + 1) % 13;

      if (frameX < 2) {
         frameX = 2;
      }

      if (frameX != lastFrameX && (frameX == 4 || frameX == 11)) {
         playSound("footstep", 0.7f);
      }

      walkTimer -= .04f;
   }
}

// Render functions

void Player::render(float accumulator) const {
   Texture2D &texture = getTexture("player");
   const Vector2 drawPos = lerp(previousPosition, position, accumulator / fixedUpdateDT);
   DrawTexturePro(texture, {frameX * playerFrameSize, 0.f, (flipX ? -playerFrameSize : playerFrameSize), (float)texture.height}, {drawPos.x, drawPos.y, playerSize.x, playerSize.y}, {0, 0}, 0, WHITE);
}

// Getter functions

Vector2 Player::getCenter() {
   return {position.x + playerSize.x / 2.f, position.y + playerSize.y / 2.f};
}

Rectangle Player::getBounds() {
   return {position.x, position.y, playerSize.x, playerSize.y};
}
