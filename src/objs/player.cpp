#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "util/math.hpp"

// Constants

constexpr Vector2 size {2.f, 3.f};
constexpr int frameSize = 20;

constexpr float updateSpeed = 1.f / 60.f;
constexpr float speed = 4.363f * .5f;
constexpr float jumpSpeed = -6.667f * .5f;
constexpr float gravity = .533f * .5f;
constexpr float maxGravity = 14.667f * .5f;
constexpr float acceleration = .083f;
constexpr float deceleration = .167f;
constexpr float smoothing = .083f * 2.f;
constexpr float jumpHoldTime = .4f;

// Constructors

void Player::init() {
   vel = {0, 0};
   prev = pos;
}

// Update functions

void Player::updatePlayer(Map& map) {
   updateTimer += GetFrameTime();
   while (updateTimer >= updateSpeed) {
      updateTimer -= updateSpeed;
      updateMovement();
      updateCollisions(map);
   }
   updateAnimation();
   prev = pos;
}

void Player::updateMovement() {
   auto dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (not onGround) {
      vel.y = min(maxGravity, vel.y + gravity);
   } else {
      vel.y = min(maxGravity, gravity);
   }

   if (dir != 0) {
      auto speedX = (onGround ? speed : speed * .6f);
      vel.x = lerp(vel.x, dir * speedX, acceleration);
   } else {
      vel.x = lerp(vel.x, 0.f, deceleration);
   }

   if (IsKeyDown(KEY_SPACE) and canHoldJump) {
      vel.y = jumpSpeed;

      holdJumpTimer += updateSpeed;
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
   vel.x *= waterMult;
   vel.y *= waterMult;

   if (not floatIsZero(vel.x)) {
      flipX = (vel.x > 0.f);
   }
}

void Player::updateCollisions(Map& map) {
   pos.y = lerp(pos.y, pos.y + vel.y, smoothing);
   bool collisionX = false, collisionY = false;
   int waterTileCount = 0;

   auto maxX = min(map.sizeX, int(pos.x + size.x) + 1);
   auto maxY = min(map.sizeY, int(pos.y + size.y) + 1);

   for (int y = max(0, (int)pos.y); y < maxY; ++y) {
      for (int x = max(0, (int)pos.x); x < maxX; ++x) {
         if (map.is(x, y, Block::air) or map.is(x, y, Block::water) or (IsKeyDown(KEY_S) and map.is(x, y, Block::platform))) {
            // Only check water tile count in the first iteration
            waterTileCount += map.is(x, y, Block::water);
            continue;
         }

         if (not CheckCollisionRecs({pos.x, pos.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (prev.y >= y + 1.f and not map.is(x, y, Block::platform)) {
            pos.y = y + 1.f;
            canHoldJump = false;
            collisionY = true;
         }

         if (prev.y + size.y <= y) {
            pos.y = y - size.y;
            onGround = true;
            collisionY = true;
         }
      }
   }

   if (not torsoCollision and feetCollision and not IsKeyDown(KEY_S)) {
      pos.y = feetCollisionY - size.y;
   }

   pos.x = lerp(pos.x, pos.x + vel.x, smoothing);

   Rectangle torso {pos.x, pos.y - 1.f, size.x, size.y};
   Rectangle feet {pos.x, pos.y + 2.f, size.x, 1.f};
   torsoCollision = feetCollision = false;
   feetCollisionY = 0;

   maxX = min(map.sizeX, int(pos.x + size.x) + 1);
   maxY = min(map.sizeY, int(pos.y + size.y) + 1);

   for (int y = max(0, (int)pos.y - 1); y < maxY; ++y) {
      for (int x = max(0, (int)pos.x); x < maxX; ++x) {
         if (map.is(x, y, Block::air) or map.is(x, y, Block::water) or map.is(x, y, Block::platform)) {
            continue;
         }

         if (not torsoCollision and CheckCollisionRecs(torso, {(float)x, (float)y, 1.f, 1.f})) {
            torsoCollision = true;
         }

         if (not feetCollision and CheckCollisionRecs(feet, {(float)x, (float)y, 1.f, 1.f})) {
            feetCollision = true;
            feetCollisionY = y;
         }

         if (not CheckCollisionRecs({pos.x, pos.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (prev.x >= x + 1.f) {
            pos.x = x + 1.f;
            collisionX = true;
         }

         if (prev.x + size.x <= x) {
            pos.x = x - size.x;
            collisionX = true;
         }
      }
   }

   waterMult = (waterTileCount > 0 ? .9f : 1.f);
   if (not collisionY) {
      onGround = false;
   }
}

void Player::updateAnimation() {
   if (not onGround) {
      fallTimer += GetFrameTime();
      if (fallTimer >= .05f) {
         fx = 5;
      }
   } else {
      fallTimer = 0.f;

      if (not floatEquals(prev.x, pos.x)) {
         walkTimer += GetFrameTime() * clamp(abs(vel.x) / speed, .1f, 1.5f);
         if (walkTimer >= .03f) {
            fx = (fx + 1) % 18;
            fx = (fx < 6 ? 6 : fx);
            walkTimer -= .03f;
         }
      } else {
         fx = 0;
      }
   }
}

// Render functions

void Player::render() {
   auto& texture = getTexture("player");
   DrawTexturePro(texture, {fx * 20.f, 0.f, (flipX ? -20.f : 20.f), (float)texture.height}, {pos.x, pos.y, size.x, size.y}, {0, 0}, 0, WHITE);
}

// Getter functions

Vector2 Player::getCenter() {
   return {pos.x + size.x / 2.f, pos.y + size.y / 2.f};
}

Rectangle Player::getBounds() {
   return {pos.x, pos.y, size.x, size.y};
}
