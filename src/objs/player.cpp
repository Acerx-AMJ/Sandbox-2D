#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "mngr/sound.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"
#include <raymath.h>

// Constants

constexpr Vector2 size {2.f, 3.f};
constexpr float frameSize = 16;

constexpr float updateSpeed = 1.f / 60.f;
constexpr float speed = 4.363f * .5f;
constexpr float debugFlySpeed = speed * 2.f;
constexpr float debugFastFlySpeed = speed * 5.f;
constexpr float jumpSpeed = -6.667f * .5f;
constexpr float gravity = .533f * .5f;
constexpr float maxGravity = 14.667f * .5f;
constexpr float acceleration = .083f;
constexpr float deceleration = .167f;
constexpr float smoothing = .083f * 2.f;
constexpr float jumpHoldTime = .4f;

// Constructors

void Player::init() {
   delta = velocity = {0, 0};
   previousPosition = position;
}

// Update functions

void Player::updatePlayer(Map &map) {
   updateTimer += GetFrameTime();
   while (updateTimer >= updateSpeed) {
      updateTimer -= updateSpeed;

      if (isDebugModeActive()) {
         updateDebugMovement();
      } else {
         updateMovement();
      }
      updateCollisions(map);
   }

   updateAnimation();
   delta = {position.x - previousPosition.x, position.y - previousPosition.y};
   previousPosition = position;
}

// Movement functions

void Player::updateMovement() {
   int directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (!onGround) {
      velocity.y = min(maxGravity, velocity.y + gravity);
   } else {
      velocity.y = min(maxGravity, gravity);
   }

   if (directionX != 0) {
      float speedX = (onGround ? speed : speed * .6f);
      velocity.x = lerp(velocity.x, directionX * speedX, acceleration * iceMultiplier);
   } else {
      velocity.x = lerp(velocity.x, 0.f, deceleration * iceMultiplier);
   }

   if (IsKeyDown(KEY_SPACE) && canHoldJump) {
      if (!justJumped) {
         playSound("jump");
         justJumped = true;
      }
      velocity.y = jumpSpeed;

      holdJumpTimer += updateSpeed;
      if (holdJumpTimer >= jumpHoldTime) {
         canHoldJump = justJumped = false;
      }
   } else {
      justJumped = false;
   }

   if (onGround) {
      canHoldJump = true;
      holdJumpTimer = 0.f;
   } else if (!IsKeyDown(KEY_SPACE)) {
      canHoldJump = false;
   }
   velocity.x *= waterMultiplier;
   velocity.y *= waterMultiplier;

   if (directionX != 0 ) {
      flipX = (directionX == 1);
   }
}

void Player::updateDebugMovement() {
   float directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
   float directionY = IsKeyDown(KEY_S) - (IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE));
   Vector2 normalized = Vector2Normalize({directionX, directionY});

   float speed = (IsKeyDown(KEY_LEFT_SHIFT) ? debugFastFlySpeed : debugFlySpeed);

   // Ice multiplier doesn't work as intended here
   velocity.x = lerp(velocity.x, normalized.x * speed, acceleration) * waterMultiplier;
   velocity.y = lerp(velocity.y, normalized.y * speed, acceleration) * waterMultiplier;

   if (directionX != 0) {
      flipX = (directionX == 1);
   }
}

// Update collisions

void Player::updateCollisions(Map &map) {
   position.y = lerp(position.y, position.y + velocity.y, smoothing);
   bool collisionY = false, canGoUpSlopes = true;
   int waterTileCount = 0, lavaTileCount = 0, iceTileCount = 0;

   if (position.y < 0) {
      position.y = 0;
      canGoUpSlopes = canHoldJump = false;
      collisionY = true;
   } else if (position.y > map.sizeY - size.y) {
      position.y = map.sizeY - size.y;
      onGround = collisionY = true;
   }

   int maxX = min(map.sizeX, int(position.x + size.x) + 1);
   int maxY = min(map.sizeY, int(position.y + size.y) + 1);

   for (int y = max(0, (int)position.y); y < maxY; ++y) {
      for (int x = max(0, (int)position.x); x < maxX; ++x) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (IsKeyDown(KEY_S) && map.isu(x, y, Block::platform))) {
            // Only check water and lava tile count in the first iteration
            waterTileCount += map.isu(x, y, Block::water);
            lavaTileCount += map.isu(x, y, Block::lava);
            continue;
         }

         if (!CheckCollisionRecs({position.x, position.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (previousPosition.y >= y + 1.f && !map.isu(x, y, Block::platform)) {
            position.y = y + 1.f;
            canHoldJump = false;
            collisionY = true;
         }

         if (previousPosition.y + size.y <= y) {
            position.y = y - size.y;
            onGround = true;
            collisionY = true;
            iceTileCount += map.isu(x, y, Block::ice);
         }
      }
   }

   if (!torsoCollision && feetCollision && !IsKeyDown(KEY_S)) {
      position.y = feetCollisionY - size.y;
   }

   position.x = lerp(position.x, position.x + velocity.x, smoothing);

   Rectangle torso {position.x, position.y - 1.f, size.x, size.y};
   Rectangle feet {position.x, position.y + 2.f, size.x, 1.f};
   torsoCollision = feetCollision = false;
   feetCollisionY = 0;

   position.x = clamp(position.x, 0.f, map.sizeX - size.x);
   maxX = min(map.sizeX, int(position.x + size.x) + 1);
   maxY = min(map.sizeY, int(position.y + size.y) + 1);

   for (int y = max(0, (int)position.y - 1); y < maxY; ++y) {
      for (int x = max(0, (int)position.x); x < maxX; ++x) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (map.isu(x, y, Block::platform) && !IsKeyDown(KEY_W))) {
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

         if (!CheckCollisionRecs({position.x, position.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (previousPosition.x >= x + 1.f) {
            position.x = x + 1.f;
         }

         if (previousPosition.x + size.x <= x) {
            position.x = x - size.x;
         }
      }
   }

   position.x = clamp(position.x, 0.f, map.sizeX - size.x);
   position.y = clamp(position.y, 0.f, map.sizeY - size.y);

   if (lavaTileCount > 0) {
      waterMultiplier = .6f;
   } else if (waterTileCount > 0) {
      waterMultiplier = .9f;
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
   if (!onGround || isDebugModeActive()) {
      fallTimer += GetFrameTime();
      if (fallTimer >= .05f) {
         frameX = 1;
      }
   } else {
      fallTimer = 0.f;

      if (previousPosition.x != position.x) {
         walkTimer += GetFrameTime() * clamp(abs(velocity.x) / speed, .1f, 1.5f);
         if (walkTimer >= .04f) {
            frameX = (frameX + 1) % 13;
            if (frameX < 2) {
               frameX = 2;
               playSound("footsteps", .7f);
            }

            walkTimer -= .04f;
         }
      } else {
         frameX = 0;
      }
   }
}

// Render functions

void Player::render() {
   Texture2D &texture = getTexture("player");
   DrawTexturePro(texture, {frameX * frameSize, 0.f, (flipX ? -frameSize : frameSize), (float)texture.height}, {position.x, position.y, size.x, size.y}, {0, 0}, 0, WHITE);
}

// Getter functions

Vector2 Player::getCenter() {
   return {position.x + size.x / 2.f, position.y + size.y / 2.f};
}

Rectangle Player::getBounds() {
   return {position.x, position.y, size.x, size.y};
}
