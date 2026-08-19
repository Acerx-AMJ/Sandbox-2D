#pragma once
#include <raylib.h>

constexpr inline float scrollBarWidth = 0.0525;

struct Scrollframe {
   void update(float dt);
   void render() const;

   // Helper functions

   void setProgressBasedOnPosition(float positionY);
   float getOffsetY() const;
   bool inFrame(const Rectangle &rect) const;

   // Members

   Rectangle rect = {0, 0, 0, 0};
   bool moving = false;

   float scrollHeight = 0.0f;
   float progress = 0.0f;
   float scrollbarY = 0.0f;
   float scrollbarHeight = 0.0f;
};
