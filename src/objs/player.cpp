#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "util/math.hpp"

// Constants

constexpr Vector2 size {2.f, 3.f};
constexpr int frameSize = 20;

constexpr float speed = 20.f;
constexpr float jumpSpeed = -8.f;
constexpr float gravity = 5.f;
constexpr float maxGravity = 55.f;
constexpr float acceleration = 9.f;
constexpr float jumpHoldTime = .3f;

// Constructors

void Player::init(const Vector2& spawnPos) {
   pos = spawnPos;
   vel = {0, 0};

   anim.tex = &ResourceManager::get().getTexture("player");
   anim.fwidth = 20;
   anim.fheight = anim.tex->height;
}

// Update functions

void Player::updatePlayer(Map& map) {
   updateMovement();
   updateCollisions(map);
   updateAnimation();
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
      anim.flipX = (vel.x > 0.f);
   }
}

void Player::updateCollisions(Map& map) {
   Vector2 original = pos;
   Rectangle bounds {pos.x + vel.x, pos.y + vel.y, size.x, size.y};
   bool collisionX = false, collisionY = false;
   int waterTileCount = 0, suffocation = 0;

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

         if (overlapX < overlapY) {
            pos.x = bounds.x = x + (x > bounds.x ? -size.x : 1.f);
            suffocation += (collisionX);
            collisionX = true;
         }
         
         if (overlapY < overlapX) {
            pos.y = bounds.y = y + (y > bounds.y ? -size.y : 1.f);
            onGround = (bounds.y + size.y <= y);
            suffocation += (collisionY);
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

   suffocating = (suffocation > 0);
   if (suffocating) {
      pos = original;
      canHoldJump = false;
   }
   waterMult = (waterTileCount > 0 ? .9f : 1.f);
   delta = {pos.x - original.x, pos.y - original.y};
}

void Player::updateAnimation() {
   if (not onGround) {
      fallTimer += GetFrameTime();
      if (fallTimer >= .05f) {
         anim.fx = 5;
      }
   } else {
      fallTimer = 0.f;

      if (not floatIsZero(delta.x)) {
         walkTimer += GetFrameTime() * clamp(abs(vel.x) / (speed * GetFrameTime()), .1f, 1.5f);
         if (walkTimer >= .04f) {
            anim.fx = ((int)anim.fx + 1) % 18;
            anim.fx = (anim.fx < 6 ? 6 : anim.fx);
            walkTimer = 0.f;
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
