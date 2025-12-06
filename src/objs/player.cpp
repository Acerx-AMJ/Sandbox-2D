#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "util/math.hpp"

// Constants

constexpr Vector2 size {2.f, 3.f};
constexpr float frameSize = 16;

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

void Player::updatePlayer(Map &map) {
   updateTimer += GetFrameTime();
   while (updateTimer >= updateSpeed) {
      updateTimer -= updateSpeed;
      updateMovement();
      updateCollisions(map);
   }
   updateAnimation();

   delta = {pos.x - prev.x, pos.y - prev.y};
   prev = pos;
}

void Player::updateMovement() {
   int dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (!onGround) {
      vel.y = min(maxGravity, vel.y + gravity);
   } else {
      vel.y = min(maxGravity, gravity);
   }

   if (dir != 0) {
      float speedX = (onGround ? speed : speed * .6f);
      vel.x = lerp(vel.x, dir * speedX, acceleration * iceMult);
   } else {
      vel.x = lerp(vel.x, 0.f, deceleration * iceMult);
   }

   if (IsKeyDown(KEY_SPACE) && canHoldJump) {
      vel.y = jumpSpeed;

      holdJumpTimer += updateSpeed;
      if (holdJumpTimer >= jumpHoldTime) {
         canHoldJump = false;
      }
   }

   if (onGround) {
      canHoldJump = true;
      holdJumpTimer = 0.f;
   } else if (!IsKeyDown(KEY_SPACE)) {
      canHoldJump = false;
   }
   vel.x *= waterMult;
   vel.y *= waterMult;

   if (dir != 0 ) {
      flipX = (dir == 1);
   }
}

void Player::updateCollisions(Map &map) {
   pos.y = lerp(pos.y, pos.y + vel.y, smoothing);
   bool collisionY = false, canGoUpSlopes = true;
   int waterTileCount = 0, lavaTileCount = 0, iceTileCount = 0;

   if (pos.y < 0) {
      pos.y = 0;
      canGoUpSlopes = canHoldJump = false;
      collisionY = true;
   } else if (pos.y > map.sizeY - size.y) {
      pos.y = map.sizeY - size.y;
      onGround = collisionY = true;
   }

   int maxX = min(map.sizeX, int(pos.x + size.x) + 1);
   int maxY = min(map.sizeY, int(pos.y + size.y) + 1);

   for (int y = max(0, (int)pos.y); y < maxY; ++y) {
      for (int x = max(0, (int)pos.x); x < maxX; ++x) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (IsKeyDown(KEY_S) && map.isu(x, y, Block::platform))) {
            // Only check water and lava tile count in the first iteration
            waterTileCount += map.isu(x, y, Block::water);
            lavaTileCount += map.isu(x, y, Block::lava);
            continue;
         }

         if (!CheckCollisionRecs({pos.x, pos.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (prev.y >= y + 1.f && !map.isu(x, y, Block::platform)) {
            pos.y = y + 1.f;
            canHoldJump = false;
            collisionY = true;
         }

         if (prev.y + size.y <= y) {
            pos.y = y - size.y;
            onGround = true;
            collisionY = true;
            iceTileCount += map.isu(x, y, Block::ice);
         }
      }
   }

   if (!torsoCollision && feetCollision && !IsKeyDown(KEY_S)) {
      pos.y = feetCollisionY - size.y;
   }

   pos.x = lerp(pos.x, pos.x + vel.x, smoothing);

   Rectangle torso {pos.x, pos.y - 1.f, size.x, size.y};
   Rectangle feet {pos.x, pos.y + 2.f, size.x, 1.f};
   torsoCollision = feetCollision = false;
   feetCollisionY = 0;

   pos.x = clamp(pos.x, 0.f, map.sizeX - size.x);
   maxX = min(map.sizeX, int(pos.x + size.x) + 1);
   maxY = min(map.sizeY, int(pos.y + size.y) + 1);

   for (int y = max(0, (int)pos.y - 1); y < maxY; ++y) {
      for (int x = max(0, (int)pos.x); x < maxX; ++x) {
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

         if (!torsoCollision && (CheckCollisionRecs(torso, {(float)x, (float)y, 1.f, 1.f}) || pos.y <= 0.f)) {
            torsoCollision = true;
         }

         if (!CheckCollisionRecs({pos.x, pos.y, size.x, size.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (prev.x >= x + 1.f) {
            pos.x = x + 1.f;
         }

         if (prev.x + size.x <= x) {
            pos.x = x - size.x;
         }
      }
   }

   pos.x = clamp(pos.x, 0.f, map.sizeX - size.x);
   pos.y = clamp(pos.y, 0.f, map.sizeY - size.y);

   if (lavaTileCount > 0) {
      waterMult = .6f;
   } else if (waterTileCount > 0) {
      waterMult = .9f;
   } else {
      waterMult = 1.f;
   }

   if (!collisionY) {
      onGround = false;
   }
   if (onGround) {
      iceMult = (iceTileCount > 0 ? .2f : 1.f);
   }
}

void Player::updateAnimation() {
   if (!onGround) {
      fallTimer += GetFrameTime();
      if (fallTimer >= .05f) {
         fx = 1;
      }
   } else {
      fallTimer = 0.f;

      if (prev.x != pos.x) {
         walkTimer += GetFrameTime() * clamp(abs(vel.x) / speed, .1f, 1.5f);
         if (walkTimer >= .04f) {
            fx = (fx + 1) % 13;
            fx = (fx < 2 ? 2 : fx);
            walkTimer -= .04f;
         }
      } else {
         fx = 0;
      }
   }
}

// Render functions

void Player::render() {
   Texture2D &texture = getTexture("player");
   DrawTexturePro(texture, {fx * frameSize, 0.f, (flipX ? -frameSize : frameSize), (float)texture.height}, {pos.x, pos.y, size.x, size.y}, {0, 0}, 0, WHITE);
}

// Getter functions

Vector2 Player::getCenter() {
   return {pos.x + size.x / 2.f, pos.y + size.y / 2.f};
}

Rectangle Player::getBounds() {
   return {pos.x, pos.y, size.x, size.y};
}
