#ifndef UI_KEYBIND_INDICATOR_HPP
#define UI_KEYBIND_INDICATOR_HPP

#include <raylib.h>
#include <string>

void drawKeybindIndicator(const std::string &keybind, const Vector2 &center, const Color &tint = WHITE);

#endif
