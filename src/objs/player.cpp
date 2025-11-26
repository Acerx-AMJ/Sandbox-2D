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

   anim.tex = &ResourceManager::get().getTexture("player");
   anim.fwidth = 20;
   anim.fheight = anim.tex->height;
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
      anim.flipX = (vel.x > 0.f);
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
         if (map.is(x, y, Block::air) or map.is(x, y, Block::water)) {
            // Only check water tile count in the first iteration
            waterTileCount += map.is(x, y, Block::water);
            continue;
         }

         if (not CheckCollisionRecs({pos.x, pos.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         bool ceilingCollision = (prev.y >= y + 1.f);
         bool floorCollision = (prev.y + size.y <= y);

         collisionY = (collisionY or ceilingCollision or floorCollision);

         if (ceilingCollision) {
            pos.y = y + 1.f;
            canHoldJump = false;
         }

         if (floorCollision) {
            pos.y = y - size.y;
            onGround = true;
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
         if (map.is(x, y, Block::air) or map.is(x, y, Block::water)) {
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

         bool leftWallCollision = (prev.x >= x + 1.f);
         bool rightWallCollision = (prev.x + size.x <= x);
         collisionX = (collisionX or leftWallCollision or rightWallCollision);

         if (leftWallCollision) {
            pos.x = x + 1.f;
         }

         if (rightWallCollision) {
            pos.x = x - size.x;
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
         anim.fx = 5;
      }
   } else {
      fallTimer = 0.f;

      if (not floatEquals(prev.x, pos.x)) {
         walkTimer += GetFrameTime() * clamp(abs(vel.x) / speed, .1f, 1.5f);
         if (walkTimer >= .03f) {
            anim.fx = ((int)anim.fx + 1) % 18;
            anim.fx = (anim.fx < 6 ? 6 : anim.fx);
            walkTimer -= .03f;
         }
      } else {
         anim.fx = 0;
      }
   }
}

// Render functions

void Player::render() {
   anim.render(pos, size);
}

// Getter functions

Vector2 Player::getCenter() {
   return {pos.x + size.x / 2.f, pos.y + size.y / 2.f};
}

Rectangle Player::getBounds() {
   return {pos.x, pos.y, size.x, size.y};
}
