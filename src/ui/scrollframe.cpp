#include "mngr/input.hpp"
#include "mngr/resource.hpp"
#include "ui/scrollframe.hpp"
#include "util/math.hpp"
#include "util/render.hpp"
#include <cmath>

// Update

void Scrollframe::update(float dt) {
   const float scrollFactor = GetMouseWheelMove();
   scrollbarHeight = rectangle.height * (rectangle.height / scrollHeight);

   if (CheckCollisionPointRec(GetMousePosition(), rectangle) && scrollFactor != 0.f) {
      progress = clamp(progress + scrollFactor * 15.0f * dt * (rectangle.height / scrollHeight), 0.f, 1.f);
   } else if (CheckCollisionPointRec(GetMousePosition(), {rectangle.x + rectangle.width - scrollBarWidth, rectangle.y, scrollBarWidth, rectangle.height})) {
      setMouseOnUI(true);
      moving = isMouseDownUI(MOUSE_BUTTON_LEFT);
   }

   // We just set 'isMouseDownUI' to false in the last if statement, do not use input manager here
   if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      moving = false;
   }

   if (moving && scrollbarHeight < rectangle.height) {
      scrollbarY = clamp<float>(GetMouseY(), rectangle.y, rectangle.y + rectangle.height - scrollbarHeight);
      progress = (scrollbarY - rectangle.y) / (rectangle.height - scrollbarHeight);
   } else {
      scrollbarY = rectangle.y + (rectangle.height - scrollbarHeight) * progress;
   }
}

// Render

void Scrollframe::render() const {
   drawTextureNoOrigin(getTexture("scrollframe"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawTextureNoOrigin(getTexture("scrollbar"), {rectangle.x + rectangle.width - scrollBarWidth, scrollbarY}, {scrollBarWidth, scrollbarHeight});
}

// Helper functions

void Scrollframe::setProgressBasedOnPosition(float positionY) {
   const float maxScroll = max(0.0f, scrollHeight - rectangle.height);
   progress = (maxScroll > 0.0f ? clamp(positionY - rectangle.y, 0.0f, maxScroll) / maxScroll : 0.0f);
   scrollbarY = rectangle.y + (rectangle.height - scrollbarHeight) * progress;
}

bool Scrollframe::inFrame(const Rectangle &rect) const {
   const float top = rectangle.y + getOffsetY();
   return rectangle.x <= rect.x && rectangle.x + rectangle.width >= rect.x + rect.width && top <= rect.y && top + rectangle.height >= rect.y + rect.height;
}

float Scrollframe::getOffsetY() const {
   return (scrollHeight - rectangle.height) * progress;
}
