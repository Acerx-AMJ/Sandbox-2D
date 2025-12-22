#include "objs/item.hpp"
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/render.hpp"
#include <raylib.h>
#include <raymath.h>

// Selected item functions

void SelectedItem::reset()  {
   item = Item{};
   address = nullptr;
   fullSelect = true;
   fromTrash = false;
}

// Dropped item functions

DroppedItem::DroppedItem(Item::Type type, unsigned char id, bool isFurniture, int count, int tileX, int tileY, float lifetime)
   : type(type), id(id), isFurniture(isFurniture), count(count), tileX(tileX), tileY(tileY), lifetime(lifetime) {}

DroppedItem::DroppedItem(Item &item, int tileX, int tileY)
   : type(item.type), id(item.id), isFurniture(item.isFurniture), count(item.count), tileX(tileX), tileY(tileY), lifetime(0.0f) {}

void DroppedItem::update(const Rectangle &cameraBounds) {
   lifetime += GetFrameTime();
   inBounds = (tileX >= cameraBounds.x && tileX <= cameraBounds.width && tileY >= cameraBounds.y && tileY <= cameraBounds.height);
}

void DroppedItem::render() const {
   if (!inBounds) {
      return;
   }

   float offsetY = std::sin(lifetime * droppedItemFloatSpeed) * droppedItemFloatHeight;

   Vector2 position = {tileX + 0.5f, (tileY + 0.5f) - offsetY};
   Vector2 size = droppedItemSize;

   if (!isFurniture) {
      drawTexture(getTexture(Block::getName(id)), position, size, 0.0f, WHITE);
   } else {
      FurnitureTexture texture = Furniture::getFurnitureIcon(id);

      if (texture.sizeX < texture.sizeY) {
         size.x *= texture.sizeX / texture.sizeY;
      } else if (texture.sizeX > texture.sizeY) {
         size.y *= texture.sizeY / texture.sizeX;
      }
      DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {position.x, position.y, size.x, size.y}, {0, 0}, 0, WHITE);
   }

   if (count != 1) {
      position.y -= 0.7f;
      drawText(position, std::to_string(count).c_str(), 0.75f, WHITE, 0.1f);
   }
}

Rectangle DroppedItem::getBounds() const {
   return {(float)tileX, (float)tileY, 1.0f, 1.0f};
}
