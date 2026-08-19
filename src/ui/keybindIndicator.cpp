#include "ui/keybindIndicator.hpp"
#include "SRU/render.hpp"
#include "SRU/util.hpp"

constexpr float fontSize = 27.5f;
constexpr Vector2 offset = {0.008f, -0.008f};
constexpr Vector2 keybindSize = {0.0278f, 0.0278f};

void drawKeybindIndicator(Font font, const std::string &keybind, Vector2 center, Color tint) {
   if (keybind.empty()) return;
   Vector2 size = mapRatioToArea(keybindSize, WINDOW_AREA, CUBIC_RATIO);
   Vector2 position = center - mapRatioToArea(offset);
   
   drawTexture("keybind", position, size, CENTER, tint);
   drawText(font, position, keybind.c_str(), getFontSizeScaled(fontSize), CENTER, BLACK);
}
