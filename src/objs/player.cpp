#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "util/math.hpp"

// Constants

constexpr Vector2 size {2.f, 3.f};

constexpr float speed = 20.f;
constexpr float jumpSpeed = -10.f;
constexpr float gravity = 5.f;
constexpr float maxGravity = 45.f;
constexpr float acceleration = 9.f;
constexpr float jumpHoldTime = .2f;

// Constructors

void Player::init(const Vector2& spawnPos) {
   pos = spawnPos;
   vel = {0, 0};
}

// Update functions

void Player::updatePlayer(Map& map) {
   updateMovement();
   updateCollisions(map);
}

void Player::updateMovement() {
   vel.y = std::min(maxGravity * GetFrameTime() * waterMult, vel.y + gravity * GetFrameTime() * waterMult);
   auto dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (dir != 0) {
      auto speedX = (onGround ? speed : speed * 1.4f);
      vel.x = lerp(vel.x, dir * speed * GetFrameTime(), acceleration * GetFrameTime());
   } else {
      vel.x = lerp(vel.x, 0.f, acceleration * GetFrameTime());
   }

   if (IsKeyDown(KEY_SPACE) and canHoldJump) {
      vel.y = std::min(0.f, vel.y);
      vel.y += jumpSpeed * GetFrameTime();

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
   vel.x *= waterMult;
   vel.y *= waterMult;

   if (not floatIsZero(vel.x)) {
      facingRight = (vel.x > 0.f);
   }
}

void Player::updateCollisions(Map& map) {
   Rectangle bounds {pos.x + vel.x, pos.y + vel.y, 2.f, 3.f};
   bool collisionX = false, collisionY = false;
   int waterTileCount = 0;

   auto maxX = std::min<int>(map[0].size(), int(bounds.x + bounds.width) + 1);
   auto maxY = std::min<int>(map.size(), int(bounds.y + bounds.height) + 1);

   for (int y = std::max(0, (int)bounds.y); y < maxY; ++y) {
      for (int x = std::max(0, (int)bounds.x); x < maxX; ++x) {
         auto& block = map[y][x];
         if (block.type == Block::Type::air or block.type == Block::Type::water) {
            waterTileCount += (block.type == Block::Type::water);
            continue;
         }

         if (not CheckCollisionRecs(bounds, {(float)x, (float)y, 1, 1})) {
            continue;
         }

         auto overlapX = std::min((pos.x + size.x) - x, (x + 1) - pos.x);
         auto overlapY = std::min((pos.y + size.y) - y, (y + 1) - pos.y);

         if (floatIsZero(overlapX) and floatIsZero(overlapY)) {
            continue;
         }

         if (overlapX < overlapY and not collisionX) {
            pos.x = bounds.x = x + (x > bounds.x ? -size.x : 1.f);
            collisionX = true;
         }
         
         if (overlapY < overlapX and not collisionY) {
            pos.y = bounds.y = y + (y > bounds.y ? -size.y : 1.f);
            onGround = (bounds.y + size.y <= y);
            collisionY = true;
         }
      }
   }

   if (not collisionX) {
      pos.x += vel.x;
   }

   if (not collisionY) {
      onGround = false;
      pos.y += vel.y;
   }
   waterMult = (waterTileCount > 0 ? .9f : 1.f);
}

// Render functions

void Player::render() {
   auto& tex = ResourceManager::get().getTexture("player");
   DrawTexturePro(tex, {0.f, 0.f, float(facingRight ? tex.width : -tex.width), (float)tex.height}, {pos.x, pos.y, size.x, size.y}, {0, 0}, 0, WHITE);
}

// Getter functions

Vector2 Player::getCenter() {
   return {pos.x + size.x / 2.f, pos.y + size.y / 2.f};
}
