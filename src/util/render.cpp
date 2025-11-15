#include "util/render.hpp"

// Includes

#include "mngr/resource.hpp"
#include "util/position.hpp"

using namespace std::string_literals;

// Render functions

void drawText(const Vector2& position, const char* text, float fontSize, float rotation) {
   auto& font = ResourceManager::get().getFont("andy"s);
   DrawTextPro(font, text, position, getOrigin(text, fontSize, 1.f), rotation, fontSize, 1.f, WHITE);
}

void drawTexture(const Texture& texture, const Vector2& position, const Vector2& size, float rotation) {
   DrawTexturePro(texture, getBox(texture), {position.x, position.y, size.x, size.y}, getOrigin(size), rotation, WHITE);
}

void drawTextureBlock(const Texture& texture, const Rectangle& rect) {
   Rectangle src {0, 0, texture.width * (rect.width / rect.height), (float)texture.height};
   DrawTexturePro(texture, src, rect, {0.f, 0.f}, 0.f, WHITE);
}

void drawRect(const Color& color) {
   DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), color);
}
