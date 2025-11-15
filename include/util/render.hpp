#ifndef UTIL_RENDER_HPP
#define UTIL_RENDER_HPP

#include <raylib.h>

// Draw functions

void drawText(const Vector2& position, const char* text, float fontSize, float rotation = 0.f);
void drawTexture(const Texture& texture, const Vector2& position, const Vector2& size, float rotation = 0.f);
void drawTextureBlock(const Texture& texture, const Rectangle& rect);
void drawRect(const Color& color);

#endif
