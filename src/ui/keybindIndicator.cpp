#include "ui/keybindIndicator.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <raymath.h>

// Constants

constexpr float fontSize      = 27.5f;
constexpr Vector2 offset      = {10.0f, -10.0f};
constexpr Vector2 keybindSize = {30.0f, 30.0f};

// Draw

void drawKeybindIndicator(const std::string &keybind, const Vector2 &center, const Color &tint) {
   if (keybind.empty()) {
      return;
   }

   const Vector2 position = Vector2Subtract(center, {offset.x * getWidthRatio(), offset.y * getHeightRatio()});
   drawTexture(getTexture("keybind"), position, {keybindSize.x * getWidthRatio(), keybindSize.y * getHeightRatio()}, 0.0f, tint);
   DrawTextPro(getFont("RobotoMono"), keybind.c_str(), position, getOrigin(MeasureTextEx(getFont("RobotoMono"), keybind.c_str(), getFontSize(fontSize), getFontSize(1.0f))), 0.0f, getFontSize(fontSize), getFontSize(1.0f), BLACK);
}
