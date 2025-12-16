#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "mngr/sound.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"
#include <raymath.h>

// Constructors

void Player::init() {
   delta = velocity = {0, 0};
   previousPosition = position;
}

// Update functions

void Player::updatePlayer(Map &map) {
   updateTimer += GetFrameTime();
   while (updateTimer >= playerUpdateSpeed) {
      updateTimer -= playerUpdateSpeed;

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
   // Hanlde gravity
   if (!onGround) {
      velocity.y = min(maxGravity, velocity.y + gravity);
   } else {
      velocity.y = min(maxGravity, gravity);
   }

   // Handle movement
   int directionX = IsKeyDown(moveRightKey) - IsKeyDown(moveLeftKey);

   if (directionX != 0) {
      float speedX = (onGround ? playerSpeed : playerSpeed * airMultiplier);
      velocity.x = lerp(velocity.x, directionX * speedX, acceleration * iceMultiplier);
   } else {
      velocity.x = lerp(velocity.x, 0.f, deceleration * iceMultiplier);
   }

   // Handle jumping
   if (!onGround && IsKeyDown(jumpKey)) {
      foxTimer = foxTime;
   } else {
      foxTimer -= playerUpdateSpeed;
   }

   if (onGround) {
      coyoteTimer = coyoteTime;
   } else {
      coyoteTimer -= playerUpdateSpeed;
   }

   if ((IsKeyDown(jumpKey) && coyoteTimer > 0) || (onGround && foxTimer > 0)) {
      playSound("jump");
      velocity.y = jumpSpeed;
      coyoteTimer = 0.f;
   }

   // Do everything else
   velocity.x *= waterMultiplier;
   velocity.y *= waterMultiplier;

   if (directionX != 0 ) {
      flipX = (directionX == 1);
   }
}

void Player::updateDebugMovement() {
   float directionX = IsKeyDown(moveRightKey) - IsKeyDown(moveLeftKey);
   float directionY = IsKeyDown(moveDownKey) - (IsKeyDown(moveUpKey) || IsKeyDown(jumpKey));
   Vector2 normalized = Vector2Normalize({directionX, directionY});

   float speed = (IsKeyDown(moveFastDebugKey) ? debugFastFlySpeed : debugFlySpeed);

   // Ice multiplier doesn't work as intended here
   velocity.x = lerp(velocity.x, normalized.x * speed, acceleration) * waterMultiplier;
   velocity.y = lerp(velocity.y, normalized.y * speed, acceleration) * waterMultiplier;

   if (directionX != 0) {
      flipX = (directionX == 1);
   }
}

// Update collisions

void Player::updateCollisions(Map &map) {
   position.y = lerp(position.y, position.y + velocity.y, playerSmoothing);
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
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (IsKeyDown(moveDownKey) && map.isu(x, y, Block::platform))) {
            // Only check water and lava tile count in the first iteration
            waterTileCount += map.isu(x, y, Block::water);
            lavaTileCount += map.isu(x, y, Block::lava);
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

   if (!torsoCollision && feetCollision && !IsKeyDown(moveDownKey)) {
      position.y = feetCollisionY - playerSize.y;
   }

   position.x = lerp(position.x, position.x + velocity.x, playerSmoothing);

   Rectangle torso {position.x, position.y - 1.f, playerSize.x, playerSize.y};
   Rectangle feet {position.x, position.y + 2.f, playerSize.x, 1.f};
   torsoCollision = feetCollision = false;
   feetCollisionY = 0;

   position.x = clamp(position.x, 0.f, map.sizeX - playerSize.x);
   maxX = min(map.sizeX, int(position.x + playerSize.x) + 1);
   maxY = min(map.sizeY, int(position.y + playerSize.y) + 1);

   for (int y = max(0, (int)position.y - 1); y < maxY; ++y) {
      for (int x = max(0, (int)position.x); x < maxX; ++x) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::water) || map.isu(x, y, Block::lava) || (map.isu(x, y, Block::platform) && !IsKeyDown(moveUpKey))) {
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
      return;
   }

   fallTimer = 0.f;
   if (previousPosition.x != position.x) {
      walkTimer += GetFrameTime() * clamp(abs(velocity.x) / playerSpeed, .1f, 1.5f);
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
   } else {
      frameX = 0;
   }
}

// Render functions

void Player::render() {
   Texture2D &texture = getTexture("player");
   DrawTexturePro(texture, {frameX * playerFrameSize, 0.f, (flipX ? -playerFrameSize : playerFrameSize), (float)texture.height}, {position.x, position.y, playerSize.x, playerSize.y}, {0, 0}, 0, WHITE);
}

// Getter functions

Vector2 Player::getCenter() {
   return {position.x + playerSize.x / 2.f, position.y + playerSize.y / 2.f};
}

Rectangle Player::getBounds() {
   return {position.x, position.y, playerSize.x, playerSize.y};
}
