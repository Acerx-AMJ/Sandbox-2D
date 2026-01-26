#ifndef OBJS_PLAYER_HPP
#define OBJS_PLAYER_HPP

#include "objs/map.hpp"
#include <limits>

constexpr int maxBreath = 100;

struct Player {
   // Constructors

   void init();

   // Update

   void updatePlayer(Map &map);
   void updateMovement();
   void updateCollisions(Map &map);
   void updateAnimation();

   // Health functions

   void takeDamage(Map &map, int damage, int critChance = 0, float critDamage = 0.0f);
   void handleRegeneration();

   // Render

   void render(float accumulator) const;

   // Getter functions

   Vector2 getCenter() const;
   Rectangle getBounds() const;

   // Members

   Vector2 position, velocity, previousPosition, delta;
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
   bool sitting = false;

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

   int lastBreakingX = 0;
   int lastBreakingY = 0;
   float breakTime = 0.0f;
};

#endif
