#include "objs/item.hpp"
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/config.hpp"
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

void DroppedItem::update(Map &map) {
   lifetime += GetFrameTime();
   if (map.empty(x, y + 2)) {
      y += GetFrameTime() * 9.f;
   }
}

void DroppedItem::render(float offsetY) {
   Vector2 position = {x + 0.5f, y + 0.5f + offsetY};
   Vector2 size = droppedItemSize;

   if (!isFurniture) {
      drawTexture(getTexture(Block::getName(id)), position, size);
   } else {
      FurnitureTexture texture = Furniture::getFurnitureIcon(id);

      if (texture.sizeX < texture.sizeY) {
         size.x *= texture.sizeX / texture.sizeY;
      } else if (texture.sizeX > texture.sizeY) {
         size.y *= texture.sizeY / texture.sizeX;
      }
      DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {position.x, position.y, size.x, size.y}, {0, 0}, 0, WHITE);
   }
}
