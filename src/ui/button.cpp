#include "mngr/input.hpp"
#include "mngr/sound.hpp"
#include "ui/button.hpp"
#include "util/math.hpp"
#include "util/render.hpp"
#include <raymath.h>

// Constants

constexpr float buttonScaleMin      = 0.98f;
constexpr float buttonScaleMax      = 1.02f;
constexpr Color buttonDisabledColor = {170, 170, 150, 255};

// Update

void Button::update(float dt, float offsetY) {
   const bool wasHovering = hovering;
   const Vector2 mousePosition = {GetMousePosition().x, GetMousePosition().y + offsetY};

   hovering = CheckCollisionPointRec(mousePosition, normalizeRect());
   if (hovering) {
      setMouseOnUI(true);
   }

   if (disabled) {
      down = false;
      clicked = false;
   } else {
      down = hovering && isMouseDownUI(MOUSE_BUTTON_LEFT);
      clicked = hovering && isMousePressedUI(MOUSE_BUTTON_LEFT);
   }

   if (down) {
      scale = max(scale - dt, buttonScaleMin);
   } else if (hovering) {
      scale = min(scale + dt, buttonScaleMax);
   } else if (scale < 1.0f) {
      scale = min(scale + dt, 1.0f);
   } else if (scale > 1.0f) {
      scale = max(scale - dt, 1.0f);
   }

   if (!wasHovering && hovering) {
      playSound("hover");
   }

   if (clicked) {
      playSound("click");
   }
}

// Render

void Button::render(float offsetY) const {
   const Color tint = disabled ? buttonDisabledColor : WHITE;

   if (texture) {
      drawTexture(*texture, {rectangle.x, rectangle.y - offsetY}, {rectangle.width * scale, rectangle.height * scale}, 0, tint);
   }
   drawText({rectangle.x, rectangle.y - offsetY}, text.c_str(), 35 * scale, tint);
}

// Get real boundaries

Rectangle Button::normalizeRect() const {
   return {rectangle.x - rectangle.width / 2.0f, rectangle.y - rectangle.height / 2.0f, rectangle.width, rectangle.height};
}
