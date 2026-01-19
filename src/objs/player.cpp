#include "game/state.hpp"
#include "mngr/resource.hpp"
#include "objs/player.hpp"
#include "mngr/sound.hpp"
#include "util/math.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
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

constexpr float coyoteTime   = 0.1f;
constexpr float foxTime      = 0.1f;
constexpr float jumpTime     = 0.25f;

constexpr float baseRegeneration         = 15.0f;
constexpr float regenerationInHoney      = 25.0f;
constexpr float immunityTime             = 0.4f;
constexpr float timeToStartRegenerating  = 15.0f;
constexpr float timeToRampUpRegeneration = 10.0f;
constexpr int framesToRegenerateOnce     = 20;

// Constructors

void Player::init() {
   displayHearts = hearts;
   delta = velocity = {0, 0};
   previousPosition = position;
}

// Update

void Player::updatePlayer(Map &map) {
   updateMovement();
   updateCollisions(map);
   updateAnimation();
   handleRegeneration();

   delta = {position.x - previousPosition.x, position.y - previousPosition.y};
   previousPosition = position;

   immunityFrame -= fixedUpdateDT;
   timeSinceLastDamage += fixedUpdateDT;
}

void Player::updateMovement() {
   int directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

   if (sitting) {
      if (directionX != 0 || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_S) || IsKeyDown(KEY_W)) {
         sitting = false;
         goto notSittingAnymore;
      }
      return;
   }
notSittingAnymore:

   // Handle gravity
   if (!onGround) {
      velocity.y += gravity;
      if (velocity.y >= maxGravity) {
         velocity.y = maxGravity;
      }
   } else if (!shouldBounce) {
      velocity.y = 0.001f; // Needed
   }

   // Handle movement
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
      coyoteTimer = 0.f;
      jumpTimer = jumpTime;

      if (shouldBounce) {
         velocity.y += jumpSpeed * 0.5f;
      } else {
         velocity.y = -jumpSpeed;
      }
   }

   if (shouldBounce) {
      shouldBounce = false;
      velocity.y = std::abs(velocity.y) * -0.8f;
   }

   // Do everything else
   velocity.x *= waterMultiplier;
   velocity.y *= std::min(1.0f, waterMultiplier * 1.6f);

   if (directionX != 0 ) {
      flipX = (directionX == 1);
   }
}

// Update collisions

void Player::updateCollisions(Map &map) {
   if (sitting) return;
   position.y += velocity.y;

   bool collisionY = false;
   bool canGoUpSlopes = true;
   int waterTileCount = 0;
   int lavaTileCount = 0;
   int honeyTileCount = 0;
   int iceTileCount = 0;

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
         if (map.isLiquidAtAll(x, y) && map.getLiquidHeight(x, y) > playerLiquidThreshold) {
            waterTileCount += map.isLiquidOfType(x, y, LiquidType::water);
            lavaTileCount  += map.isLiquidOfType(x, y, LiquidType::lava);
            honeyTileCount += map.isLiquidOfType(x, y, LiquidType::honey);
         }
         honeyTileCount += map.isu(x, y, BlockType::sticky);

         if ((!map.isu(x, y, BlockType::solid) && !map.isPlatformedFurniture(x, y)) || ((map.isu(x, y, BlockType::platform) || map.isu(x, y, BlockType::furnitureTop)) && IsKeyDown(KEY_S))) {
            continue;
         }

         if (!CheckCollisionRecs({position.x, position.y, playerSize.x, playerSize.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (previousPosition.y >= y + 1.f && !map.isu(x, y, BlockType::platform)) {
            velocity.y = max(0.f, velocity.y);
            position.y = y + 1.f;
            collisionY = true;
            onGround = false;
         }

         if (previousPosition.y + playerSize.y <= y) {
            shouldBounce = (!onGround && map.isu(x, y, BlockType::bouncy) && std::abs(velocity.y) > 0.5f);
            iceTileCount += map.isu(x, y, BlockType::ice);

            position.y = y - playerSize.y;
            onGround = true;
            collisionY = true;
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
         // Necessary to count in both loops for making the player stick to sticky walls
         if (map.isLiquidAtAll(x, y) && map.getLiquidHeight(x, y) > playerLiquidThreshold) {
            waterTileCount += map.isLiquidOfType(x, y, LiquidType::water);
            lavaTileCount  += map.isLiquidOfType(x, y, LiquidType::lava);
            honeyTileCount += map.isLiquidOfType(x, y, LiquidType::honey);
         }
         honeyTileCount += map.isu(x, y, BlockType::sticky);

         if ((!map.isu(x, y, BlockType::solid) && !map.isPlatformedFurniture(x, y)) || ((map.isu(x, y, BlockType::platform) || map.isu(x, y, BlockType::furnitureTop)) && !IsKeyDown(KEY_W))) {
            continue;
         }

         if (canGoUpSlopes && !feetCollision && CheckCollisionRecs(feet, {(float)x, (float)y, 1.f, 1.f})) {
            feetCollision = true;
            feetCollisionY = y;
         }

         if (map.isu(x, y, BlockType::platform) || map.isu(x, y, BlockType::furnitureTop)) {
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

   if (honeyTileCount > 0) {
      waterMultiplier = 0.5f;
   } else if (lavaTileCount > 0) {
      waterMultiplier = .6f;
   } else if (waterTileCount > 0) {
      waterMultiplier = .85f;
   } else {
      waterMultiplier = 1.f;
   }

   if (lavaTileCount > 0) {
      takeDamage(random(25, 45));
   }

   regeneration = (honeyTileCount > 0 ? regenerationInHoney : baseRegeneration);

   if (!collisionY) {
      onGround = false;
   }
   if (onGround) {
      iceMultiplier = (iceTileCount > 0 ? .2f : 1.f);
   }
}

void Player::updateAnimation() {
   if (sitting) {
      frameX = 15;
      return;
   }

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

// Health functions

void Player::takeDamage(int damage) {
   if (immunityFrame > 0.0f) {
      return;
   }
   hearts = max(0, hearts - damage);
   immunityFrame = immunityTime;
   timeSinceLastDamage = timeSpentRegenerating = 0.0f;
}

void Player::handleRegeneration() {
   displayHearts = lerp(displayHearts, hearts, 0.2f);

   if (hearts == maxHearts) {
      timeSinceLastDamage = timeSpentRegenerating = 0.0f;
      displayHearts = maxHearts;
      return;
   }

   if (timeSinceLastDamage < timeToStartRegenerating) {
      return;
   }

   timeSpentRegenerating += fixedUpdateDT;
   regenerationFrameCounter = (regenerationFrameCounter + 1) % framesToRegenerateOnce;
   if (regenerationFrameCounter != 0) {
      return;
   }

   hearts = min<int>(maxHearts, hearts + regeneration * min(1.0f, timeSpentRegenerating / timeToRampUpRegeneration));
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
