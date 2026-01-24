#include "objs/item.hpp"
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/render.hpp"
#include <raylib.h>
#include <raymath.h>
#include <cmath>

// Constants

constexpr int droppedItemLifetime      = 60.0f * 15.0f;
constexpr float droppedItemFloatSpeed  = 1.5f;
constexpr float droppedItemFloatHeight = 0.25f;
constexpr Vector2 droppedItemSize      = {0.8f, 0.8f};

// Selected item functions

void SelectedItem::reset()  {
   item = Item{};
   address = nullptr;
   fullSelect = true;
   fromTrash = false;
}

// Dropped item functions

DroppedItem::DroppedItem(ItemType type, unsigned short id, unsigned short count, bool isFurniture, bool isWall, int tileX, int tileY, float lifetime)
   : type(type), id(id), count(count), lifetime(lifetime), tileX(tileX), tileY(tileY), isFurniture(isFurniture), isWall(isWall) {}

DroppedItem::DroppedItem(Item &item, int tileX, int tileY)
   : type(item.type), id(item.id), count(item.count), lifetime(0.0f), tileX(tileX), tileY(tileY), isFurniture(item.isFurniture), isWall(item.isWall) {}

void DroppedItem::update(const Rectangle &cameraBounds, float dt) {
   lifetime += dt;
   inBounds = (tileX >= cameraBounds.x && tileX <= cameraBounds.width && tileY >= cameraBounds.y && tileY <= cameraBounds.height);

   if (lifetime >= droppedItemLifetime) {
      flagForDeletion = true;
   }
}

void DroppedItem::render() const {
   if (!inBounds) {
      return;
   }

   float offsetY = std::sin(lifetime * droppedItemFloatSpeed) * droppedItemFloatHeight;

   Color drawColor = (isWall ? wallTint : WHITE);
   Vector2 position = {tileX + 0.5f, (tileY + 0.5f) - offsetY};
   Vector2 size = droppedItemSize;

   if (!isFurniture) {
      drawTexture(getTexture(getBlockNameFromId(id)), position, size, 0.0f, drawColor);
   } else {
      FurnitureTexture texture = getFurnitureIcon(id);

      if (texture.sizeX < texture.sizeY) {
         size.x *= texture.sizeX / texture.sizeY;
      } else if (texture.sizeX > texture.sizeY) {
         size.y *= texture.sizeY / texture.sizeX;
      }
      DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {position.x, position.y, size.x, size.y}, Vector2Scale(size, 0.5f), 0, drawColor);
   }

   if (count != 1) {
      position.y -= 0.7f;
      drawText(position, std::to_string(count).c_str(), 0.75f, WHITE, 0.1f);
   }
}

Rectangle DroppedItem::getBounds() const {
   return {(float)tileX, (float)tileY, 1.0f, 1.0f};
}
