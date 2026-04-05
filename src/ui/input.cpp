#include "mngr/input.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/input.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <cmath>

// Constants

constexpr float textWrapPadding = 10.0f;
constexpr float fadeSpeed       = 0.3f / 0.016f;
constexpr int   fadeMin         = 200;
constexpr int   fadeRange       = 255 - fadeMin;

// Helper functions

static bool consumeBackspace(std::string &text, size_t &cursor) {
   if (cursor == 0) {
      return false;
   }

   cursor -= 1;
   text.erase(text.begin() + cursor);

   if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
      while (cursor != 0 && !std::isspace(text[cursor - 1])) {
         cursor -= 1;
         text.erase(text.begin() + cursor);
      }
   }
   return true;
}

// Update

void Input::update(float dt) {
   if (prevsize != text.size()) {
      cursor = text.size();
   }
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

   if (typing && (isKeyRepeated(KEY_LEFT))) {
      cursor = (cursor == 0 ? cursor : cursor - 1);

      if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
         while (cursor != 0 && !std::isspace(text[cursor - 1])) {
            cursor -= 1;
         }
      }
      rendercursor = true;
      cursorcounter = 0.0f;
   } else if (typing && (isKeyRepeated(KEY_RIGHT))) {
      cursor = (cursor == text.size() ? cursor : cursor + 1);

      if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
         while (cursor != text.size() && !std::isspace(text[cursor - 1])) {
            cursor += 1;
         }
      }
      rendercursor = true;
      cursorcounter = 0.0f;
   }

   if (typing) {
      const std::size_t previousTextSize = text.size();

      if (isKeyRepeated(KEY_BACKSPACE) || isKeyRepeated(KEY_DELETE)) {
         changed = consumeBackspace(text, cursor);
      }

      for (char c = GetCharPressed(); c != 0 && (int)text.size() < maxChars; c = GetCharPressed()) {
         cursor = std::min(cursor, text.size());
         text.insert(text.begin() + cursor, c);
         changed = true;
         cursor += 1;
      }

      if (text.size() != previousTextSize) {
         playSound("typing");
      }
   }

   if (wasTyping != typing) {
      playSound("click");
   }

   if (!wasTyping && typing) {
      text.clear();
      changed = true;
   }

   counter += dt;
   cursorcounter += dt;

   if (cursorcounter >= 0.5f) {
      cursorcounter -= 0.5f;
      rendercursor = !rendercursor;
   }
   prevsize = text.size();
}

// Render function

void Input::render() {
   float fontsize = getFontSize(35.0f);
   float spacing = getFontSize(1.0f);
   unsigned char value = 255;

   if (typing) {
      value = std::sin(counter * fadeSpeed) * fadeRange + fadeMin;
   }

   if (text.empty()) {
      value -= fadeRange;
   }

   if (texture) {
      drawTexture(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   }

   std::string selected = text.empty() ? fallback : text;
   if (wrapinput) {
      wrapText(selected, rectangle.width - textWrapPadding, fontsize, spacing);
      drawText({rectangle.x, rectangle.y}, selected.c_str(), fontsize, Color{value, value, value, 255}, spacing);
   } else {
      Vector2 origin = getOrigin(selected.c_str(), fontsize, spacing);
      Vector2 position = {rectangle.x - (origin.x - rectangle.width / 2.0f), rectangle.y};
      DrawTextPro(getFont("andy"), selected.c_str(), position, origin, 0, fontsize, spacing, Color{value, value, value, 255});

      if (rendercursor && !text.empty()) {
         std::string substr = selected.substr(0, cursor);
         Vector2 cursorPosition = MeasureTextEx(getFont("andy"), (substr.empty() ? "X" : substr.c_str()), fontsize, spacing);
         if (substr.empty()) {
            cursorPosition.x = 0.0f;
         }

         DrawRectangleV(Vector2Add({cursorPosition.x, 0}, Vector2Subtract(position, origin)), {fontsize * (15.0f / 35.0f), fontsize}, Fade(WHITE, 0.75f));
      }
   }
}

// Normalize rect

Rectangle Input::normalizeRect() const {
   return {rectangle.x - rectangle.width / 2.0f, rectangle.y - rectangle.height / 2.0f, rectangle.width, rectangle.height};
}
