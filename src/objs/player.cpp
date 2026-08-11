#include "game/state.hpp"
#include "objs/player.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/random.hpp"
#include <raymath.h>

// Player's keybinds shouldn't overlap with any other keybinds in GameState. It
// should be non-blocking, so our input manager is not used here.

// Constants

constexpr Vector2 playerSize    = {1.8f, 2.7f};
constexpr float playerFrameSizeX = 16;
constexpr float playerFrameSizeY = 24;

constexpr float playerSpeed   = 0.364f;
constexpr float flySpeed      = 0.6f;
constexpr float fastFlySpeed  = 1.2f;
constexpr float airMultiplier = 0.600f;
constexpr float jumpSpeed     = 0.950f;
constexpr float gravity       = 0.028f;
constexpr float maxGravity    = 1.333f;
constexpr float acceleration  = 0.083f;
constexpr float deceleration  = 0.167f;

constexpr float coyoteTime   = 0.1f;
constexpr float foxTime      = 0.1f;
constexpr float jumpTime     = 0.25f;

constexpr float immunityTime             = 0.4f;
constexpr float timeToStartRegenerating  = 15.0f;
constexpr float timeToRampUpRegeneration = 10.0f;
constexpr float regenSpeedInHoney        = 1.8f;
constexpr int   framesToRegenerateOnce   = 20;
constexpr int   framesToUpdateBreath     = 10;

constexpr float minimumFallHeight = 35.0f;
constexpr float maximumFallHeight = 110.0f;
constexpr float maximumFallDamage = 500.0f;

// Constructors

void Player::init() {
   lastHearts = displayHearts = hearts;
   displayBreath = breath;
   
   timeSinceLastDamage = immunityTime; // Prevent player from spawning in red
   delta = velocity = {0, 0};
   previousPosition = position;
}

// Update

void Player::updatePlayer(Map &map) {
   lastHearts = hearts;
   
   updateMovement();
   updateCollisions(map);
   updateAnimation();
   handleRegeneration();

   delta = {position.x - previousPosition.x, position.y - previousPosition.y};
   previousPosition = position;

   immunityFrame -= fixedUpdateDT;
   timeSinceLastDamage += fixedUpdateDT * regenSpeedMultiplier;

   displayHearts = Lerp(displayHearts, float(hearts), 0.3f);
   displayBreath = Lerp(displayBreath, float(breath), 0.3f);
}

void Player::updateMovement() {
   if (creative) {
      maximumY = position.y;

      float dirx = (!blockInput && IsKeyDown(KEY_D)) - (!blockInput && IsKeyDown(KEY_A));
      float diry = (!blockInput && IsKeyDown(KEY_S)) - (!blockInput && IsKeyDown(KEY_W));
      float speed = (IsKeyDown(KEY_LEFT_SHIFT) ? fastFlySpeed : flySpeed);
      Vector2 normalized = Vector2Normalize({dirx, diry});

      // Do not give a fuck about ice while flying
      if (dirx != 0) {
         velocity.x = Lerp(velocity.x, normalized.x * speed, acceleration);
      } else {
         velocity.x = Lerp(velocity.x, 0.0f, deceleration);
      }

      if (diry != 0) {
         velocity.y = Lerp(velocity.y, normalized.y * speed, acceleration);
      } else {
         velocity.y = Lerp(velocity.y, 0.0f, deceleration);
      }

      // Do everything else
      velocity.x *= waterMultiplier;
      velocity.y *= waterMultiplier;

      if (dirx != 0) {
         flipX = (dirx == 1);
      }
      return;
   }

   int directionX = (!blockInput && IsKeyDown(KEY_D)) - (!blockInput && IsKeyDown(KEY_A));

   // Handle gravity
   if (!onGround) {
      float maxWaterMultiplier = (waterMultiplier != 1.0f ? waterMultiplier * 0.2f : waterMultiplier);
      velocity.y += gravity;

      if (velocity.y >= maxGravity * maxWaterMultiplier) {
         velocity.y = maxGravity * maxWaterMultiplier;
      }
   } else if (!shouldBounce) {
      velocity.y = 0.001f; // Needed. Why?
   }

   // Handle movement
   if (directionX != 0) {
      float speedX = (onGround ? playerSpeed : playerSpeed * airMultiplier);
      velocity.x = Lerp(velocity.x, directionX * speedX, acceleration * iceMultiplier);
   } else {
      velocity.x = Lerp(velocity.x, 0.f, deceleration * iceMultiplier);
   }

   // Handle jumping
   if (!onGround && (!blockInput && IsKeyDown(KEY_SPACE))) {
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
   if ((((!blockInput && IsKeyDown(KEY_SPACE)) && coyoteTimer > 0) || (onGround && foxTimer > 0)) && jumpTimer <= 0) {
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
   // velocity.y *= std::min(1.0f, waterMultiplier * 1.6f);

   if (directionX != 0 ) {
      flipX = (directionX == 1);
   }
}

// Update collisions

void Player::updateCollisions(Map &map) {
   if (ignoreCollision) {
      ignoreCollision = false;
      return;
   }

   position.y += velocity.y;

   bool wasOnGround = onGround;
   bool collisionY = false;
   bool canGoUpSlopes = true;
   int liquidsAboveHead = 0;
   int blocksInHeadX1 = 0;
   int blocksInHeadX2 = 0;
   int waterTileCount = 0;
   int lavaTileCount = 0;
   int honeyTileCount = 0;
   int iceTileCount = 0;

   if (position.y < 0) {
      velocity.y = std::max(0.f, velocity.y);
      position.y = 0;
      canGoUpSlopes = onGround = false;
      collisionY = true;
   } else if (position.y > map.sizeY - playerSize.y) {
      position.y = map.sizeY - playerSize.y;
      onGround = collisionY = true;
   }

   int maxX = std::min(map.sizeX, int(position.x + playerSize.x) + 1);
   int maxY = std::min(map.sizeY, int(position.y + playerSize.y) + 1);

   for (int y = std::max(0, (int)position.y); y < maxY; ++y) {
      for (int x = std::max(0, (int)position.x); x < maxX; ++x) {
         if (map.isAnyLiquid(x, y) && map.getLiquidHeight(x, y) > playerLiquidThreshold) {
            waterTileCount += map.isLiquidOfType(x, y, LiquidType::water);
            lavaTileCount  += map.isLiquidOfType(x, y, LiquidType::lava);
            honeyTileCount += map.isLiquidOfType(x, y, LiquidType::honey);
            liquidsAboveHead += (y <= position.y + 1.0f);
         }
         honeyTileCount += map.is(x, y, BlockType::sticky);

         if (!map.is(x, y, BlockType::solid) || ((map.is(x, y, BlockType::platform) || map.blocks[y * map.sizeX + x].platformOverride) && (!blockInput && IsKeyDown(KEY_S)))) {
            continue;
         }

         // Not necessary in both loops
         blocksInHeadX1 += (y <= position.y + 1.0f && x >= position.x + playerSize.x / 2.0f && !map.is(x, y, BlockType::platform) && !map.blocks[y * map.sizeX + x].platformOverride);
         blocksInHeadX2 += (y <= position.y + 1.0f && x <  position.x + playerSize.x / 2.0f && !map.is(x, y, BlockType::platform) && !map.blocks[y * map.sizeX + x].platformOverride);

         if (!CheckCollisionRecs({position.x, position.y, playerSize.x, playerSize.y}, {(float)x, (float)y, 1.f, 1.f})) {
            continue;
         }

         if (previousPosition.y >= y + 1.f && !map.is(x, y, BlockType::platform)) {
            velocity.y = std::max(0.f, velocity.y);
            position.y = y + 1.f;
            collisionY = true;
            onGround = false;
         }

         if (previousPosition.y + playerSize.y <= y) {
            shouldBounce = (!onGround && map.is(x, y, BlockType::bouncy) && std::abs(velocity.y) > 0.5f);
            iceTileCount += map.is(x, y, BlockType::ice);

            position.y = y - playerSize.y;
            onGround = true;
            collisionY = true;
         }
      }
   }

   if (!torsoCollision && feetCollision && !(!blockInput && IsKeyDown(KEY_S))) {
      position.y = feetCollisionY - playerSize.y;
   }

   position.x += velocity.x;

   Rectangle torso {position.x, position.y - 1.f, playerSize.x, playerSize.y};
   Rectangle feet {position.x, position.y + (playerSize.y - 1.f), playerSize.x, 1.f};
   torsoCollision = feetCollision = false;
   feetCollisionY = 0;

   position.x = Clamp(position.x, 0.f, map.sizeX - playerSize.x);
   maxX = std::min(map.sizeX, int(position.x + playerSize.x) + 1);
   maxY = std::min(map.sizeY, int(position.y + playerSize.y) + 1);

   for (int y = std::max(0, (int)position.y - 1); y < maxY; ++y) {
      for (int x = std::max(0, (int)position.x); x < maxX; ++x) {
         // Necessary to count in both loops for making the player stick to sticky walls
         if (map.isAnyLiquid(x, y) && map.getLiquidHeight(x, y) > playerLiquidThreshold) {
            waterTileCount += map.isLiquidOfType(x, y, LiquidType::water);
            lavaTileCount  += map.isLiquidOfType(x, y, LiquidType::lava);
            honeyTileCount += map.isLiquidOfType(x, y, LiquidType::honey);
            liquidsAboveHead += (y <= position.y + 1.0f);
         }
         honeyTileCount += map.is(x, y, BlockType::sticky);

         if (!map.is(x, y, BlockType::solid) || ((map.is(x, y, BlockType::platform) || map.blocks[y * map.sizeX + x].platformOverride))) {
            continue;
         }

         if (canGoUpSlopes && !feetCollision && CheckCollisionRecs(feet, {(float)x, (float)y, 1.f, 1.f})) {
            feetCollision = true;
            feetCollisionY = y;
         }

         if (map.is(x, y, BlockType::platform) || map.blocks[y * map.sizeX + x].platformOverride) {
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

   position.x = Clamp(position.x, 0.f, map.sizeX - playerSize.x);
   position.y = Clamp(position.y, 0.f, map.sizeY - playerSize.y);

   // Apply damage to the player

   if (honeyTileCount > 0 && !onGround) {
      maximumY = std::min(maximumY, position.y);
   }

   // Ignore fall damage if player is touching liquids or didn't fall
   // from that great of a height
   if (!creative && !wasOnGround && onGround && !shouldBounce && honeyTileCount + lavaTileCount + waterTileCount == 0 && position.y - maximumY >= minimumFallHeight) {
      takeDamage(map, std::min(1.0f, ((position.y - maximumY) - minimumFallHeight) / (maximumFallHeight - minimumFallHeight)) * maximumFallDamage, 0, 0.0f);
   }

   breathFrameCounter = (breathFrameCounter + 1) % framesToUpdateBreath;
   if (!creative && breathFrameCounter == 0) {
      if (liquidsAboveHead || (blocksInHeadX1 && blocksInHeadX2)) {
         breath = std::max(0, breath - 2);
      } else {
         breath = std::min(maxBreath, breath + 1);
      }

      if (breath == 0) {
         takeDamage(map, randomInt(1, 6), 0, 0.0f);
      }
   }

   if (honeyTileCount > 0) {
      waterMultiplier = 0.5f;
   } else if (lavaTileCount > 0) {
      waterMultiplier = .6f;
   } else if (waterTileCount > 0) {
      waterMultiplier = .85f;
   } else {
      waterMultiplier = 1.f;
   }

   if (!creative && lavaTileCount > 0) {
      takeDamage(map, randomInt(20, 30), 25, 1.2f);
   }

   // Get other things right

   regenSpeedMultiplier = (honeyTileCount > 0 ? regenSpeedInHoney : 1.0f);
   if (!collisionY) {
      onGround = false;
   }

   if (onGround) {
      iceMultiplier = (iceTileCount > 0 ? 0.2f : 1.0f);
      maximumY = std::numeric_limits<float>::max();
   } else {
      maximumY = std::min(maximumY, position.y);
      iceMultiplier = 1.0f;
   }
}

void Player::updateAnimation() {
   if (creative) {
      frameX = 1;
      return;
   }

   if (breakingBlock || placedBlock) {
      breakAnimationTimer += fixedUpdateDT;
      if (breakAnimationTimer >= 0.1f) {
         breakAnimationTimer -= 0.1f;
         breakAnimation = (breakAnimation + 1) % 3;
         placedBlock = placedBlock && breakAnimation != 0;
      }
   } else {
      breakAnimation = 0;
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

   walkTimer += Clamp(abs(velocity.x) / playerSpeed, 0.1f, 1.5f) * fixedUpdateDT;
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

void Player::takeDamage(Map &map, int damage, int critChance, float critDamage) {
   if (immunityFrame > 0.0f) {
      return;
   }
   playSound("hurt");
   bool critical = chance(critChance);
   int damageApplied = damage * (critical ? critDamage : 1.0f);

   hearts = std::max(0, hearts - damageApplied);
   immunityFrame = immunityTime;
   timeSinceLastDamage = timeSpentRegenerating = 0.0f;
   // map.addDamageIndicator(getCenter(), damageApplied, critical);
}

void Player::handleRegeneration() {
   if (hearts == maxHearts) {
      return;
   }

   if (timeSinceLastDamage < timeToStartRegenerating) {
      return;
   }

   timeSpentRegenerating += fixedUpdateDT * regenSpeedMultiplier;
   regenerationFrameCounter = (regenerationFrameCounter + 1) % framesToRegenerateOnce;
   if (regenerationFrameCounter != 0) {
      return;
   }

   hearts = std::min<int>(maxHearts, hearts + regeneration * std::min(1.0f, timeSpentRegenerating / timeToRampUpRegeneration));
}

// Render functions

void Player::render(float accumulator, Texture2D *itemTexture) const {
   Texture2D &texture = getTexture("player");
   const Vector2 drawPos = Vector2Lerp(previousPosition, position, accumulator / fixedUpdateDT);

   DrawTexturePro(texture, {frameX * playerFrameSizeX, 0.f, (flipX ? -playerFrameSizeX : playerFrameSizeX), playerFrameSizeY}, {drawPos.x, drawPos.y, playerSize.x, playerSize.y}, {0, 0}, 0, (timeSinceLastDamage <= 0.3f ? RED : WHITE));
   DrawTexturePro(texture, {playerFrameSizeX * (breakingBlock || placedBlock ? 16 + breakAnimation : frameX), playerFrameSizeY, (flipX ? -playerFrameSizeX : playerFrameSizeX), playerFrameSizeY}, {drawPos.x, drawPos.y, playerSize.x, playerSize.y}, {0, 0}, 0, (timeSinceLastDamage <= 0.3f ? RED : WHITE));

   // Hard-coded tool animation 
   if (breakingBlock && itemTexture) {
      if (breakAnimation == 0) {
         DrawTexturePro(*itemTexture, {0, 0, (float)itemTexture->width, (float)itemTexture->height}, {drawPos.x + ((flipX ? 12.0f : 13.0f) / 8.0f) - flipX, drawPos.y + (5.5f / 8.0f), 1.0f, 1.0f}, {1.0f, 1.0f}, 45.0f, WHITE);
      } else if (breakAnimation == 1) {
         DrawTexturePro(*itemTexture, {0, 0, (float)itemTexture->width, (float)itemTexture->height}, {drawPos.x + ((flipX ? 15.0f : 10.0f) / 8.0f) - flipX, drawPos.y + (8.0f / 8.0f) - flipX, 1.0f, 1.0f}, {(float)!flipX, 1.0f}, (flipX ? 90.0f : 0.0f), WHITE);
      } else {
         DrawTexturePro(*itemTexture, {0, 0, (float)itemTexture->width, (float)itemTexture->height}, {drawPos.x + ((flipX ? 20.0f : 7.0f) / 8.0f) - flipX, drawPos.y + (16.0f / 8.0f) - (flipX * 0.7f), 1.0f, 1.0f}, {(float)!flipX, 1.0f}, (flipX ? 135.0f : -45.0f), WHITE);
      }
   }
}

// Getter functions

Vector2 Player::getCenter() const {
   return {position.x + playerSize.x / 2.f, position.y + playerSize.y / 2.f};
}

Rectangle Player::getBounds() const {
   return {position.x, position.y, playerSize.x, playerSize.y};
}
