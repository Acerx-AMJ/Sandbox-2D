#ifndef OBJS_PLAYER_HPP
#define OBJS_PLAYER_HPP

#include "mngr/animation.hpp"
#include "objs/generation.hpp" // IWYU pragma: export

// Player

struct Player {
   AnimationManager anim;
   Vector2 pos, vel, delta;
   bool onGround = false;
   bool canHoldJump = true;
   bool suffocating = false;
   float waterMult = 1.f;
   float holdJumpTimer = 0.f;

   float fallTimer = 0.f;
   float walkTimer = 0.f;
   int walkFrame = 6;

   // Constructors

   void init(const Vector2& spawnPos);

   // Update functions

   void updatePlayer(Map& map);
   void updateMovement();
   void updateCollisions(Map& map);
   void updateAnimation();

   // Render function

   void render();

   // Getter functions

   Vector2 getCenter();
};

#endif
