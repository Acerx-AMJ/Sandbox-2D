#include "objs/item.hpp"
#include "objs/inventory.hpp"
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
   Vector2 position = {tileX + 0.5f, (tileY + 0.5f) - offsetY};
   Vector2 size = droppedItemSize;
   drawItem(type, id, count, isFurniture, isWall, position, size, false, true);
}

Rectangle DroppedItem::getBounds() const {
   return {(float)tileX, (float)tileY, 1.0f, 1.0f};
}
