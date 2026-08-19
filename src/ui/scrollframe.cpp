#include "mngr/input.hpp"
#include "ui/scrollframe.hpp"
#include "SRU/render.hpp"
#include <raymath.h>

void Scrollframe::update(float dt) {
   const float scrollFactor = GetMouseWheelMove();
   scrollbarHeight = rect.height * (rect.height / scrollHeight);

   if (CheckCollisionPointRec(GetMousePosition(), rect) && scrollFactor != 0.f) {
      progress = Clamp(progress + scrollFactor * 15.0f * dt * (rect.height / scrollHeight), 0.f, 1.f);
   } else if (CheckCollisionPointRec(GetMousePosition(), {rect.x + rect.width - scrollBarWidth, rect.y, scrollBarWidth, rect.height})) {
      setMouseOnUI(true);
      moving = isMouseDownUI(MOUSE_BUTTON_LEFT);
   }

   // We just set 'isMouseDownUI' to false in the last if statement, do not use input manager here
   if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      moving = false;
   }

   if (moving && scrollbarHeight < rect.height) {
      scrollbarY = Clamp(GetMouseY(), rect.y, rect.y + rect.height - scrollbarHeight);
      progress = (scrollbarY - rect.y) / (rect.height - scrollbarHeight);
   } else {
      scrollbarY = rect.y + (rect.height - scrollbarHeight) * progress;
   }
}

void Scrollframe::render() const {
   float width = mapRatioToWidth(scrollBarWidth, WINDOW_AREA, CUBIC_RATIO);
   drawTexture("scrollframe", rect, TOP_LEFT);
   drawTexture("scrollbar", Vector2{rect.x + rect.width - width}, {width, scrollbarHeight}, TOP_LEFT);
}

void Scrollframe::setProgressBasedOnPosition(float positionY) {
   const float maxScroll = std::max(0.0f, scrollHeight - rect.height);
   progress = (maxScroll > 0.0f ? Clamp(positionY - rect.y, 0.0f, maxScroll) / maxScroll : 0.0f);
   scrollbarY = rect.y + (rect.height - scrollbarHeight) * progress;
}

bool Scrollframe::inFrame(const Rectangle &rect2) const {
   const float top = rect.y + getOffsetY();
   return rect.x <= rect2.x && rect.x + rect.width >= rect2.x + rect2.width && top <= rect2.y && top + rect.height >= rect2.y + rect2.height;
}

float Scrollframe::getOffsetY() const {
   return (scrollHeight - rect.height) * progress;
}
