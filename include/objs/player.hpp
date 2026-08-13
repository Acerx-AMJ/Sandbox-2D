#pragma once
#include "objs/map.hpp"
#include <limits>

constexpr int maxBreath = 100;

struct Player {
   void init();

   // update

   void updatePlayer(Map &map);
   void updateMovement();
   void updateCollisions(Map &map);
   void updateAnimation();

   // interaction

   void takeDamage(int damage, int critChance = 0, float critDamage = 0.0f);
   void handleRegeneration();
   void placeBlock();

   // render

   void render(float accumulator) const;

   // getters functions

   Vector2 getCenter() const;
   Rectangle getBounds() const;

   // Mmembers

   std::vector<int> liquidCounts;
   Vector2 position, spawnPos, velocity, previousPosition, delta;
   bool blockInput = false;
   bool feetCollision = false;
   bool torsoCollision = false;
   int feetCollisionY = 0;

   bool onGround = false;
   bool shouldBounce = false;
   float coyoteTimer = 0.f;
   float foxTimer = 0.f;
   float maximumY = std::numeric_limits<float>::max();

   float waterMultiplier = 1.f;
   float iceMultiplier = 1.f;

   float fallTimer = 0.f;
   float walkTimer = 0.f;
   float jumpTimer = 0.f;

   int walkFrame = 6;
   int frameX = 0;
   bool flipX = false;
   bool ignoreCollision = false;

   int breathFrameCounter = 0;
   int breath = maxBreath;

   int lastHearts = 100;
   int hearts = 100;
   int maxHearts = 100;
   int regenerationFrameCounter = 0;
   float immunityFrame = 0.0f;
   float timeSinceLastDamage = 0.0f;
   float timeSpentRegenerating = 0.0f;
   float regenSpeedMultiplier = 1.0f;
   float regeneration = 15.0f;

   float displayHearts = 100;
   float displayBreath = 100;

   bool placedBlockAnimation = false;
   bool canPlaceBlock = true;
   float blockPlacementSpeed = 0.1f;
   float blockPlacementTimer = 0.0f;

   int breakAnimation = 0;
   int lastBreakingX = 0;
   int lastBreakingY = 0;
   bool breakingBlock = false;
   float breakTime = 0.0f;
   float breakAnimationTimer = 0.0f;

   bool creative = false;
};
