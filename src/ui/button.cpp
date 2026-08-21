#include "mngr/input.hpp"
#include "ui/button.hpp"
#include "ui/keybindIndicator.hpp"
#include "SRU/audio.hpp"
#include "SRU/render.hpp"
#include "SRU/util.hpp"
#include <raymath.h>

constexpr float buttonScaleMin = 0.98f;
constexpr float buttonScaleMax = 1.02f;
constexpr Color buttonDisabledTint = {170, 170, 150, 255};

void Button::init(Font font, Texture texture, Vector2 origin, const std::string &text, const std::string &keybind) {
   this->font = font;
   this->texture = texture;
   this->origin = origin;
   this->text = text;
   this->keybind = keybind;
}

void Button::update(float dt) {
   bool wasHovering = hovering;
   hovering = CheckCollisionPointRec(GetMousePosition(), R4bounds(rect, origin));
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
      scale = std::max(scale - dt, buttonScaleMin);
   } else if (hovering) {
      scale = std::min(scale + dt, buttonScaleMax);
   } else if (scale < 1.0f) {
      scale = std::min(scale + dt, 1.0f);
   } else if (scale > 1.0f) {
      scale = std::max(scale - dt, 1.0f);
   }

   if (!wasHovering && hovering) {
      playSound("hover");
   }

   if (clicked) {
      playSound("click");
   }
}

void Button::render() {
   Color tint = (disabled ? buttonDisabledTint : WHITE);
   Vector2 size = R4size(rect) * scale;

   if (texture.id != 0) {
      drawTexture(texture, R4pos(rect), size, origin, tint);
   }
   drawText(font, R4anchor(rect, origin, CENTER), text.c_str(), getFontSizeScaled(35.0f * scale), CENTER, tint);
   drawKeybindIndicator(font, keybind, R4anchor(rect, origin, TOP_RIGHT), tint);
}
