#ifndef UTIL_RENDER_HPP
#define UTIL_RENDER_HPP

#include <raylib.h>

void drawText(const Vector2 &position, const char *text, float fontSize, const Color &color = WHITE, float spacing = 1.0f);
void drawTexture(const Texture &texture, const Vector2 &position, const Vector2 &size, float rotation = 0.f, const Color &color = WHITE);
void drawTextureNoOrigin(const Texture &texture, const Vector2 &position, const Vector2 &size, const Color &color = WHITE);
void drawTextureBlock(const Texture &texture, const Rectangle &rect, const Color &color = WHITE);
void drawRect(const Color &color);

#endif
