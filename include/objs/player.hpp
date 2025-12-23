#ifndef OBJS_PLAYER_HPP
#define OBJS_PLAYER_HPP

#include "objs/map.hpp"

struct Player {
   // Constructors

   void init();

   // Update

   void updatePlayer(Map &map);
   void updateMovement();
   void updateCollisions(Map &map);
   void updateAnimation();

   // Render

   void render() const;

   // Getter functions

   Vector2 getCenter();
   Rectangle getBounds();

   // Members

   Vector2 position, velocity, previousPosition, delta;
   bool feetCollision = false;
   bool torsoCollision = false;
   int feetCollisionY = 0;

   bool onGround = false;
   float coyoteTimer = 0.f;
   float foxTimer = 0.f;

   float waterMultiplier = 1.f;
   float iceMultiplier = 1.f;

   float updateTimer = 0.f;
   float fallTimer = 0.f;
   float walkTimer = 0.f;
   int walkFrame = 6;
   bool flipX = false;
   int frameX = 0;
};

#endif
