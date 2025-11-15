#include "mngr/resource.hpp"
#include "util/position.hpp"

// Screen position functions

Vector2 getScreenSize(float offsetX, float offsetY) {
   return {offsetX + GetScreenWidth(), offsetY + GetScreenHeight()};
}

Vector2 getScreenCenter(float offsetX, float offsetY) {
   return {offsetX + GetScreenWidth() / 2.f, offsetY + GetScreenHeight() / 2.f};
}

// Origin functions

Vector2 getOrigin(const Vector2& size) {
   return {size.x / 2.f, size.y / 2.f};
}

Vector2 getOrigin(float x, float y) {
   return {x / 2.f, y / 2.f};
}

Vector2 getOrigin(const char* text, float fontSize, float spacing) {
   return getOrigin(MeasureTextEx(ResourceManager::get().getFont("andy"), text, fontSize, spacing));
}

// Texture functions

Vector2 getSize(const Texture& texture) {
   return {(float)texture.width, (float)texture.height};
}

Rectangle getBox(const Texture& texture) {
   return {0.f, 0.f, (float)texture.width, (float)texture.height};
}

// Camera functions

Rectangle getCameraBounds(const Camera2D& camera) {
   Vector2 pos = GetScreenToWorld2D({0, 0}, camera);
   return {pos.x, pos.y, GetScreenWidth() / camera.zoom, GetScreenHeight() / camera.zoom};
}
