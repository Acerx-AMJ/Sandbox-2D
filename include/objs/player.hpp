#ifndef OBJS_PLAYER_HPP
#define OBJS_PLAYER_HPP

// Includes

#include "objs/generation.hpp"

// Player

struct Player {
   Vector2 pos, vel;
   bool onGround = false;
   bool canHoldJump = true;
   bool facingRight = true;
   float waterMult = 1.f;
   float holdJumpTimer = 0.f;

   // Constructors

   void init(const Vector2& spawnPos);

   // Update functions

   void updatePlayer(Map& map);
   void updateMovement();
   void updateCollisions(Map& map);

   // Render function

   void render();

   // Getter functions

   Vector2 getCenter();
};

#endif
