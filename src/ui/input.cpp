#include "mngr/input.hpp"
#include "mngr/sound.hpp"
#include "ui/input.hpp"
#include "util/format.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <cmath>

// Constants

constexpr float textWrapPadding = 10.0f;
constexpr float fadeSpeed       = 18.0f;
constexpr int   fadeMin         = 200;
constexpr int   fadeRange       = 255 - fadeMin;

// Helper functions

static bool consumeBackspace(std::string &text) {
   if (text.empty()) {
      return false;
   }

   text.pop_back();
   if (IsKeyDown(KEY_LEFT_CONTROL)) {
      while (!text.empty() && !std::isspace(text.back())) {
         text.pop_back();
      }
   }
   return true;
}

// Update

void Input::update() {
   changed = false;

   const bool wasTyping = typing;
   hovering = CheckCollisionPointRec(GetMousePosition(), normalizeRect());

   if (hovering) {
      setMouseOnUI(true);
   }

   if (hovering && isMousePressedUI(MOUSE_BUTTON_LEFT)) {
      typing = !typing;
   } else if (typing && (isKeyPressed(KEY_ENTER) || isKeyPressed(KEY_ESCAPE) || isMousePressed(MOUSE_BUTTON_LEFT))) {
      typing = false;
   }

   if (typing) {
      const std::size_t previousTextSize = text.size();

      if (isKeyRepeated(KEY_BACKSPACE) || isKeyRepeated(KEY_DELETE)) {
         changed = consumeBackspace(text);
      }

      for (char c = GetCharPressed(); c != 0 && (int)text.size() < maxChars; c = GetCharPressed()) {
         text += c;
         changed = true;
      }

      if (text.size() != previousTextSize) {
         playSound("hover");
      }
   }

   if (wasTyping != typing) {
      playSound("click");
   }

   if (!wasTyping && typing) {
      text.clear();
      changed = true;
   }
   ++counter;
}

// Render function

void Input::render(float dt) const {
   unsigned char value = 255;
   if (typing) {
      value = std::sin(counter * fadeSpeed * dt) * fadeRange + fadeMin;
   }

   if (text.empty()) {
      value -= fadeRange;
   }

   std::string wrapped = text.empty() ? fallback : text;
   wrapText(wrapped, rectangle.width - textWrapPadding, 35, 1);

   if (texture) {
      drawTexture(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   }
   drawText({rectangle.x, rectangle.y}, wrapped.c_str(), 35, Color{value, value, value, 255});
}

// Normalize rect

Rectangle Input::normalizeRect() const {
   return {rectangle.x - rectangle.width / 2.0f, rectangle.y - rectangle.height / 2.0f, rectangle.width, rectangle.height};
}
