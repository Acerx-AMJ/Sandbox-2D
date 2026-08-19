#include "mngr/input.hpp"
#include "ui/input.hpp"
#include "SRU/audio.hpp"
#include "SRU/render.hpp"
#include "SRU/text.hpp"
#include "SRU/util.hpp"
#include <algorithm>

constexpr float textWrapPadding = 0.0015f;
constexpr float fadeSpeed = 0.3f / 0.016f;
constexpr int fadeMin = 200;
constexpr int fadeRange = 255 - fadeMin;

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

void Input::init(Font font, Texture texture, Vector2 origin, int maxChars, const std::string &fallback) {
   this->font = font;
   this->texture = texture;
   this->origin = origin;
   this->maxChars = maxChars;
   this->fallback = fallback;
}

void Input::update(float dt) {
   if (prevsize != text.size()) {
      cursor = text.size();
   }
   changed = false;

   const bool wasTyping = typing;
   hovering = CheckCollisionPointRec(GetMousePosition(), R4bounds(rect, origin));

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

void Input::render() {
   std::string selected = text.empty() ? fallback : text;
   float fontsize = getFontSizeScaled(35.0f);
   unsigned char value = 255;

   if (typing) {
      value = std::sin(counter * fadeSpeed) * fadeRange + fadeMin - (fadeRange * text.empty());
   }

   if (texture.id != 0) {
      drawTexture(texture, rect, origin);
   }

   if (wrapinput) {
      wrapInPlace(selected, font, rect.width - mapRatioToWidth(textWrapPadding, WINDOW_AREA, CUBIC_RATIO), fontsize);
   }

   if (typing && rendercursor && !text.empty()) {
      // kind of hacky. we need to translate cursor for wrapInPlace and account for any new dashes inserted into the text.
      // we just check if there's a dash followed by a newline and if so then increment it by 2.
      size_t wrappedCursor = cursor;
      for (size_t i = 0; i < wrappedCursor; ++i) {
         wrappedCursor += (selected[i] == '-' && i + 1 < selected.size() && selected[i + 1] == '\n') * 2;
      }

      std::string before = selected.substr(0, wrappedCursor);
      size_t find = before.find('\n');
      size_t lineStart = 0;
      int lineIndex = 0;
   
      while (find != std::string::npos) {
         lineStart = find + 1;
         lineIndex += 1;
         find = before.find('\n', lineStart);
      }

      Vector2 textTopleft = R4anchor(rect, origin, textOrigin) - getTextOrigin(font, selected.c_str(), fontsize, textOrigin);
      float lineWidth = getTextSize(font, before.substr(lineStart).c_str(), fontsize).x;
      float lineHeight = lineIndex * getTextSize(font, "X", fontsize).y;
      drawRect(V2(lineWidth, lineHeight) + textTopleft, V2(fontsize * (15.0f / 35.0f), fontsize), TOP_LEFT, Fade(WHITE, 0.75f));
   }
   drawText(font, R4anchor(rect, origin, textOrigin), selected.c_str(), fontsize, textOrigin, Color{value, value, value, 255});
}
