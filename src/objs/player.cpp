#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "util/math.hpp"

// Constants

constexpr Vector2 size {2.f, 3.f};
constexpr int frameSize = 20;

constexpr float speed = 20.f;
constexpr float jumpSpeed = -27.5f;
constexpr float gravity = 2.5f;
constexpr float maxGravity = 55.f;
constexpr float acceleration = 5.f;
constexpr float deceleration = 10.f;
constexpr float jumpHoldTime = .4f;

// Constructors

void Player::init(const Vector2& spawnPos) {
   pos = spawnPos;
   vel = {0, 0};
   prev = {0, 0};

   anim.tex = &ResourceManager::get().getTexture("player");
   anim.fwidth = 20;
   anim.fheight = anim.tex->height;
}

// Update functions

void Player::updatePlayer(Map& map) {
   updateMovement();
   updateCollisions(map);
   updateAnimation();
   prev = pos;
}

void Player::updateMovement() {
   auto dt = GetFrameTime();
   auto dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (not onGround) {
      vel.y = std::min(maxGravity * dt, vel.y + gravity * dt);
   } else {
      vel.y = 0.f;
   }

   if (dir != 0) {
      auto speedX = (onGround ? speed : speed * .6f);
      vel.x = lerp(vel.x, dir * speedX * dt, acceleration * dt);
   } else {
      vel.x = lerp(vel.x, 0.f, deceleration * dt);
   }

   if (IsKeyDown(KEY_SPACE) and canHoldJump) {
      vel.y = jumpSpeed * dt;

      holdJumpTimer += dt;
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
   pos.y += vel.y;
   bool collisionX = false, collisionY = false;
   int waterTileCount = 0;

   auto maxX = min((int)map[0].size(), int(pos.x + size.x) + 1);
   auto maxY = min((int)map.size(), int(pos.y + size.y) + 1);

   for (int y = max(0, (int)pos.y); y < maxY; ++y) {
      for (int x = max(0, (int)pos.x); x < maxX; ++x) {
         if (map[y][x].type == Block::Type::air or map[y][x].type == Block::Type::water) {
            // Only check water tile count in the first iteration
            waterTileCount += (map[y][x].type == Block::Type::water);
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

   pos.x += vel.x;
   maxX = min((int)map[0].size(), int(pos.x + size.x) + 1);
   maxY = min((int)map.size(), int(pos.y + size.y) + 1);

   for (int y = max(0, (int)pos.y); y < maxY; ++y) {
      for (int x = max(0, (int)pos.x); x < maxX; ++x) {
         if (map[y][x].type == Block::Type::air or map[y][x].type == Block::Type::water) {
            continue;
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
         walkTimer += GetFrameTime() * clamp(abs(vel.x) / (speed * GetFrameTime()), .1f, 1.5f);
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
