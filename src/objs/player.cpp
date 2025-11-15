#include "objs/player.hpp"

// Includes

#include "util/math.hpp"

// Constants

constexpr Vector2 size {2.f, 3.f};

constexpr float speed = 20.f;
constexpr float jumpSpeed = -20.f;
constexpr float gravity = 15.f;
constexpr float maxGravity = 45.f;
constexpr float acceleration = 16.f;
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
   vel.y = std::min(maxGravity * GetFrameTime(), vel.y + gravity * GetFrameTime());
   auto dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (dir != 0) {
      auto speedX = (onGround ? speed : speed * 1.4f);
      vel.x = lerp(vel.x, dir * speed * GetFrameTime(), acceleration * GetFrameTime());
   } else {
      vel.x = lerp(vel.x, 0.0, acceleration * GetFrameTime());
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
}

void Player::updateCollisions(Map& map) {
   Rectangle bounds {pos.x + vel.x, pos.y + vel.y, 2.f, 3.f};
   bool collisionX = false, collisionY = false;

   auto maxY = std::min<int>(map.size(), int((bounds.y + bounds.height)) + 1);
   auto maxX = std::min<int>(map[0].size(), int((bounds.x + bounds.width)) + 1);

   for (int y = std::max(0, int(bounds.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(bounds.x)); x < maxX; ++x) {
         auto& block = map[y][x];
         if (block.type == Block::Type::air or block.type == Block::Type::water) {
            continue;
         }

         if (not CheckCollisionRecs(bounds, {(float)x, (float)y, 1, 1})) {
            continue;
         }
         auto overlapX = std::min((pos.x + 2.f) - x, (x + 1) - pos.x);
         auto overlapY = std::min((pos.y + 3.f) - y, (y + 1) - pos.y);

         if (overlapX == 0.f and overlapY == 0.f) {
            continue;
         }

         if (overlapX < overlapY) {
            if (x > pos.x) {
               pos.x = x - 2.f;
            } else {
               pos.x = x + 1.f;
            }
            collisionX = true;
         } else {
            if (y > pos.y) {
               pos.y = y - 3.f;
               onGround = true;
            } else {
               pos.y = y + 1.f;
            }
            collisionY = true;
         }

         if (collisionX and collisionY) {
            return;
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
}

// Render functions

void Player::render() {
   DrawRectanglePro({pos.x, pos.y, size.x, size.y}, {0, 0}, 0, RED);
}

// Getter functions

Vector2 Player::getCenter() {
   return {pos.x + size.x / 2.f, pos.y + size.y / 2.f};
}
