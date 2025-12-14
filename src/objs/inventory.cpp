#include "objs/inventory.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "objs/map.hpp"
#include "util/render.hpp"
#include <raymath.h>

void Inventory::update() {
   // Handle opening inventory
   if (IsKeyReleased(toggleInventoryKey)) {
      playSound("click");
      open = !open;
   }

   // Handle switching
   int lastSelectedX = selectedX;
   int lastSelectedY = selectedY;

   // Gotta do what you gotta do
   if (IsKeyReleased(KEY_ONE)) {
      selectedX = 0;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_TWO)) {
      selectedX = 1;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_THREE)) {
      selectedX = 2;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_FOUR)) {
      selectedX = 3;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_FIVE)) {
      selectedX = 4;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_SIX)) {
      selectedX = 5;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_SEVEN)) {
      selectedX = 6;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_EIGHT)) {
      selectedX = 7;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_NINE)) {
      selectedX = 8;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_ZERO)) {
      selectedX = 9;
      selectedY = 0;
   }

   float wheel = GetMouseWheelMove();
   if (wheel >= 1.0f) {
      selectedX = (selectedX + 1) % 10;
   } else if (wheel <= -1.0f) {
      selectedX = (selectedX == 0 ? 9 : selectedX - 1);
   }

   if (selectedX != lastSelectedX || selectedY != lastSelectedY) {
      playSound("hover");
   }

   // WARNING: scary and hard-to-track logic. Using a lot of returns and GOTO statements. This whole file is pretty
   // scary and you shouldn't be here for long. If it works, don't touch it.

   // Handle dragging
   Vector2 mousePosition = GetMousePosition();
   bool shouldDiscard = false;
   
   for (int y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {(float)x, (float)y}), itemframeTopLeft);
         Vector2 size = itemframeSize;

         if (x == selectedX && y == selectedY) {
            position = Vector2Subtract(position, selectedItemFrameOffset);
            size = selectedItemFrameSize;
         }

         Rectangle rect {position.x, position.y, size.x, size.y};
         if (!CheckCollisionPointRec(mousePosition, rect)) {
            continue;
         }

         Item &item = items[y][x];

         // Handle favoriting items
         if (IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && open && item.id != 0) {
            playSound("click");
            item.favorite = !item.favorite;
            return;
         }

         // Handle trashing items
         if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && open && item.id != 0) {
            if (item.favorite) {
               return;
            }

            // TODO: add trash sound
            wasTrashed = false;
            anyTrashed = true;
            trashedItem = std::move(item);
            item = Item{};
            return;
         }

         // When pressing on frames while the inventory is closed, select the item
         if (y == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (!open) {
               playSound("click");
            }
            selectedX = x;
            selectedY = 0;
         }

         // Handle swapping/discarding items
         if (open && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && anySelected) {
            playSound("click");

            if (&item == selectedItem) {
               shouldDiscard = true;
               goto breakOut;
            } else {
               if (wasTrashed) {
                  anyTrashed = (item.id != 0);
               }
               std::swap(item, *selectedItem);
               wasTrashed = false;
               anySelected = false;
               selectedItem = nullptr;
            }
            return;
         }

         // Handle dragging items around
         if (open && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !anySelected && item.id != 0) {
            playSound("click");
            anySelected = true;
            selectedItem = &item;
            return;
         }
      }
   }

   // Handle trash frame
   if (open) {
      Vector2 trashPosition = Vector2Add(Vector2Multiply(itemframePadding, {(float)inventoryWidth - 1, (float)inventoryHeight}), itemframeTopLeft);
      Vector2 trashSize = itemframeSize;
      Rectangle trashRect {trashPosition.x, trashPosition.y, trashSize.x, trashSize.y};

      if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || !CheckCollisionPointRec(mousePosition, trashRect)) {
         goto breakOut;
      }

      // Trash the item
      if (anySelected) {
         if (selectedItem->favorite) {
            shouldDiscard = true;
            goto breakOut;
         }

         playSound("click");
         anyTrashed = true;
         trashedItem = *selectedItem;

         if (!wasTrashed) {
            *selectedItem = Item{};
         }
         selectedItem = nullptr;
         anySelected = false;
         return;
      }

      // Un-trash the item
      if (!anySelected && anyTrashed) {
         playSound("click");
         anySelected = true;
         wasTrashed = true;
         selectedItem = &trashedItem;
         anyTrashed = false;
         return;
      }
   }
   breakOut:

   // Discard items here
   bool pressedOutside = (anySelected && open && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
   if (shouldDiscard || (anySelected && !open) || pressedOutside) {
      if (wasTrashed) {
         trashedItem = *selectedItem;
         anyTrashed = true;
      }

      if (pressedOutside) {
         playSound("click");
      }

      wasTrashed = false;
      anySelected = false;
      selectedItem = nullptr;
   }
}

// Render functions

void Inventory::render() {
   for (int y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {(float)x, (float)y}), itemframeTopLeft);
         Item &item = items[y][x];

         if (x == selectedX && y == selectedY) {
            drawTextureNoOrigin(getTexture((item.favorite ? "small_frame_favorite_selected" : "small_frame_selected")), Vector2Subtract(position, selectedItemFrameOffset), selectedItemFrameSize);
         } else {
            drawTextureNoOrigin(getTexture((item.favorite ? "small_frame_favorite" : "small_frame")), position, itemframeSize);
         }

         if (!anySelected || &item != selectedItem) {
            renderItem(item, position);
         }

         if (y == 0) {
            Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
            drawText(textPosition, std::to_string(x + 1).c_str(), 25);
         }
      }
   }

   // Render trash frame if inventory is open
   if (open) {
      Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {(float)inventoryWidth - 1, (float)inventoryHeight}), itemframeTopLeft);
      drawTextureNoOrigin(getTexture((anyTrashed ? "small_frame" : "small_frame_trash")), position, itemframeSize);

      if (anyTrashed) {
         renderItem(trashedItem, position);
      }
   }

   // Render selected item
   if (anySelected) {
      renderItem(*selectedItem, GetMousePosition());
   }
}

void Inventory::renderItem(Item &item, const Vector2 &position) {
   bool isItemValid = (item.id != 0);
   Color drawColor = (anySelected &&& item == selectedItem ? Fade(WHITE, 0.75f) : WHITE);

   if (isItemValid && !item.isFurniture) {
      drawTextureNoOrigin(getTexture(Block::getName(item.id)), Vector2Add(position, itemframeItemOffset), itemframeItemSize, drawColor);
   } else if (isItemValid && item.isFurniture) {
      FurnitureTexture texture = Furniture::getFurnitureIcon(item.id);
      Vector2 newPos = Vector2Add(position, Vector2Scale(itemframeSize, 0.5f));
      Vector2 fSize = itemframeItemSize;

      if (texture.sizeX < texture.sizeY) {
         fSize.x *= texture.sizeX / texture.sizeY;
      } else if (texture.sizeX > texture.sizeY) {
         fSize.y *= texture.sizeY / texture.sizeX;
      }
      DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {newPos.x, newPos.y, fSize.x, fSize.y}, Vector2Scale(fSize, 0.5f), 0, drawColor);
   }

   if (isItemValid && item.count > 1) {
      Vector2 textPosition = Vector2Subtract(Vector2Add(position, itemframeSize), itemframeIndexOffset);
      drawText(textPosition, std::to_string(item.count).c_str(), 25, drawColor);
   }
}
