#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

void drawText(const Vector2 &position, const char *text, float fontSize, const Color &color) {
   DrawTextPro(getFont("andy"), text, position, getOrigin(text, fontSize, 1), 0, fontSize, 1, color);
}

void drawTexture(const Texture &texture, const Vector2 &position, const Vector2 &size, float rotation, const Color &color) {
   DrawTexturePro(texture, getBox(texture), {position.x, position.y, size.x, size.y}, getOrigin(size), rotation, color);
}

void drawTextureNoOrigin(const Texture &texture, const Vector2 &position, const Vector2 &size) {
   DrawTexturePro(texture, getBox(texture), {position.x, position.y, size.x, size.y}, {0, 0}, 0, WHITE);
}

void drawTextureBlock(const Texture &texture, const Rectangle &rect, const Color &color) {
   Rectangle source {0, 0, texture.width * (rect.width / rect.height), (float)texture.height};
   DrawTexturePro(texture, source, rect, {0, 0}, 0, color);
}

void drawParallaxTexture(const Texture &texture, float &progress, float speed) {
   Vector2 screenSize = getScreenSize();
   progress -= speed * GetFrameTime();
   
   if (progress <= -screenSize.x) {
      progress = 0.f;
   }

   drawTextureNoOrigin(texture, {progress, 0}, screenSize);
   drawTextureNoOrigin(texture, {screenSize.x + progress, 0}, screenSize);
}

void drawRect(const Color &color) {
   DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), color);
}
