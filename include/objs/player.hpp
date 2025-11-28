#ifndef OBJS_PLAYER_HPP
#define OBJS_PLAYER_HPP

#include "objs/map.hpp"

// Player

struct Player {
   Vector2 pos, vel, prev;
   bool onGround = false;
   bool canHoldJump = true;
   bool feetCollision = false;
   bool torsoCollision = false;
   bool flipX = false;
   int feetCollisionY = 0;
   float waterMult = 1.f;
   float holdJumpTimer = 0.f;

   float updateTimer = 0.f;
   float fallTimer = 0.f;
   float walkTimer = 0.f;
   int walkFrame = 6;
   int fx = 0;

   // Constructors

   void init();

   // Update functions

   void updatePlayer(Map& map);
   void updateMovement();
   void updateCollisions(Map& map);
   void updateAnimation();

   // Render function

   void render();

   // Getter functions

   Vector2 getCenter();
   Rectangle getBounds();
};

#endif
