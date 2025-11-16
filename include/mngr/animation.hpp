#ifndef MNGR_ANIMATION_HPP
#define MNGR_ANIMATION_HPP

#include <raylib.h>

// Animation manager

struct AnimationManager {
   Texture* tex;
   float fwidth = 0;
   float fheight = 0;
   float fx = 0;
   float fy = 0;
   bool flipX = false;
   bool flipY = false;

   void render(const Vector2& pos, const Vector2& size);
};

#endif
