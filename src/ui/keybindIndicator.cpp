#include "ui/keybindIndicator.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <raymath.h>

// Constants

constexpr float fontSize  = 25.0f;
constexpr Color textColor = {36, 37, 77, 255};

constexpr Vector2 offset      = {10.0f, -10.0f};
constexpr Vector2 keybindSize = {30.0f, 30.0f};

// Draw

void drawKeybindIndicator(const std::string &keybind, const Vector2 &center, const Color &tint) {
   if (keybind.empty()) {
      return;
   }

   const Vector2 position = Vector2Subtract(center, offset);
   drawTexture(getTexture("keybind"), position, keybindSize, 0.0f, tint);
   DrawTextPro(getFont("RobotoMono"), keybind.c_str(), position, getOrigin(MeasureTextEx(getFont("RobotoMono"), keybind.c_str(), fontSize, 1.0f)), 0.0f, fontSize, 1.0f, textColor);
}
