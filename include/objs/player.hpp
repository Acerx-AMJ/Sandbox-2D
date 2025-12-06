#ifndef OBJS_PLAYER_HPP
#define OBJS_PLAYER_HPP

#include "objs/map.hpp"

// Player

struct Player {
   Vector2 position, velocity, previousPosition, delta;
   bool onGround = false, canHoldJump = true;
   bool feetCollision = false, torsoCollision = false;
   int feetCollisionY = 0;

   float waterMultiplier = 1.f, iceMultiplier = 1.f;
   float holdJumpTimer = 0.f;

   float updateTimer = 0.f;
   float fallTimer = 0.f;
   float walkTimer = 0.f;
   int walkFrame = 6;
   bool flipX = false;
   int frameX = 0;

   // Constructors

   void init();

   // Update functions

   void updatePlayer(Map &map);
   void updateMovement();
   void updateDebugMovement();
   void updateCollisions(Map &map);
   void updateAnimation();

   // Render function

   void render();

   // Getter functions

   Vector2 getCenter();
   Rectangle getBounds();
};

#endif
