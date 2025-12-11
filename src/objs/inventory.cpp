#include "objs/inventory.hpp"
#include "mngr/resource.hpp"
#include "util/render.hpp"
#include <raymath.h>

void Inventory::render() {
   for (float y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (float x = 0; x < inventoryWidth; ++x) {
         Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {x, y}), itemframeTopLeft);
         drawTextureNoOrigin(getTexture("small_frame"), position, itemframeSize);

         if (y == 0) {
            Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
            drawText(textPosition, std::to_string((int)x + 1).c_str(), 25);
         }
      }
   }
}
