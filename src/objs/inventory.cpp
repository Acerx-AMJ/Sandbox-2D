#include "objs/inventory.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "objs/map.hpp"
#include "util/input.hpp"
#include "util/math.hpp"
#include "util/render.hpp"
#include <raymath.h>

// Constructors

Inventory::Inventory(Map &map, Player &player, std::vector<DroppedItem> &droppedItems)
   : map(map), player(player), droppedItems(droppedItems) {}

// Update functions

void Inventory::update() {
   toggleInventoryOpen();

   // Handle switching
   int lastSelectedX = selectedX;
   int lastSelectedY = selectedY;

   switchOnKeyPress(KEY_ONE,   0);
   switchOnKeyPress(KEY_TWO,   1);
   switchOnKeyPress(KEY_THREE, 2);
   switchOnKeyPress(KEY_FOUR,  3);
   switchOnKeyPress(KEY_FIVE,  4);
   switchOnKeyPress(KEY_SIX,   5);
   switchOnKeyPress(KEY_SEVEN, 6);
   switchOnKeyPress(KEY_EIGHT, 7);
   switchOnKeyPress(KEY_NINE,  8);
   switchOnKeyPress(KEY_ZERO,  9);

   switchOnMouseWheel();

   if (selectedX != lastSelectedX || selectedY != lastSelectedY) {
      playSound("hover");
   }

   // Handle item operations
   int height = (open ? inventoryHeight : 1);
 
   for (int y = 0; y < height; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         bool isSelected = (x == selectedX && y == selectedY);
         Item &item = items[y][x];
         
         Vector2 position = getFramePosition(x, y, isSelected);
         Vector2 size = getFrameSize(isSelected);

         if (!mouseOnFrame(position, size)) {
            continue;
         }
         setMouseOnUI(true);
         bool mousePressed = isMousePressedUI(MOUSE_BUTTON_LEFT);
         bool shouldReset = true;

         // Handle favoriting items
         if (mousePressed && IsKeyDown(KEY_LEFT_ALT) && open && item.id != 0) {
            playSound("click");
            item.favorite = !item.favorite;
            return;
         }

         // Handle trashing items
         if (mousePressed && IsKeyDown(KEY_LEFT_CONTROL) && open && item.id != 0) {
            if (item.favorite) {
               return;
            }

            playSound("trash");
            trashedItem = item;
            item = Item{};
            return;
         }

         // When pressing on frames while the inventory is closed, select the item
         if (mousePressed && y == 0) {
            if (!open) {
               playSound("click");
            }
            selectedX = x;
            selectedY = 0;
            shouldReset = false;
         }

         // Handle swapping/discarding items
         if (mousePressed && open && anySelected) {
            playSound("click");

            if (&item == selectedItem.address || (selectedItem.fromTrash && item.favorite)) {
               discardItem();
               return;
            }

            if (selectedItem.fullSelect) {
               if (item.id == selectedItem.item.id) {
                  addItemCount(item, *selectedItem.address);
                  item.favorite = (item.favorite || selectedItem.item.favorite);
               } else {
                  std::swap(item, *selectedItem.address);
               }
            } else {
               if (item.id == 0) {
                  selectedItem.item.favorite = false;
                  item = selectedItem.item;
               } else if (item.id != selectedItem.item.id) {
                  discardItem();
                  return;
               } else if (addItemCount(item, selectedItem.item) > 0 && !placeItem(selectedItem.item)) {
                  dropSelectedItem();
                  return;
               }
            }
            anySelected = false;
            selectedItem.reset();
            return;
         }

         // Handle dragging items around
         if (mousePressed && open && !anySelected && item.id != 0) {
            playSound("click");
            anySelected = true;
            selectedItem = {item, &item, true, false};
            return;
         }

         if (shouldReset) {
            resetMouseUIInput(MOUSE_BUTTON_LEFT);
         }

         if (open && item.id != 0 && isMousePressedUI(MOUSE_BUTTON_RIGHT)) {
            if (!anySelected) {
               selectedItem = {item, &item, false, false};
               selectedItem.item.count = 1;
            } else if (item.id != selectedItem.item.id || getItemStackSize(selectedItem.item) <= selectedItem.item.count) {
               return;
            } else {
               selectedItem.item.favorite = (selectedItem.item.favorite || item.favorite);
               selectedItem.item.count += 1;
            }

            item.count -= 1;
            if (item.count <= 0) {
               item = Item{};
            }

            playSound("click");
            anySelected = true;
            return;
         }
      }
   }

   // Handle trash frame
   if (open) {
      Vector2 trashPosition = getFramePosition(inventoryWidth - 1, inventoryHeight, false);
      Vector2 trashSize = getFrameSize(false);

      if (!mouseOnFrame(trashPosition, trashSize)) {
         handleDiscarding();
         return;
      }
      setMouseOnUI(true);
      bool mousePressed = isMousePressedUI(MOUSE_BUTTON_LEFT);

      // Handle shift-clicking
      if (IsKeyDown(KEY_LEFT_SHIFT) && mousePressed && trashedItem.id != 0) {
         if (placeItem(trashedItem)) {
            trashedItem = Item{};
         }
         playSound("click");
         return;
      }

      // Trash the item
      if (mousePressed && anySelected) {
         if (selectedItem.item.favorite) {
            discardItem();
            return;
         }

         if (selectedItem.fullSelect) {
            *selectedItem.address = Item{};
         }

         playSound("trash");
         trashedItem = selectedItem.item;
         selectedItem.reset();
         anySelected = false;
         return;
      }

      // Un-trash the item
      if (mousePressed && !anySelected && trashedItem.id != 0) {
         playSound("click");
         anySelected = true;
         selectedItem = {trashedItem, &trashedItem, true, true};
         return;
      }

      resetMouseUIInput(MOUSE_BUTTON_LEFT);
      if (trashedItem.id != 0 && isMousePressedUI(MOUSE_BUTTON_RIGHT)) {
         if (!anySelected) {
            selectedItem = {trashedItem, &trashedItem, false, false};
            selectedItem.item.count = 1;
         } else if (trashedItem.id != selectedItem.item.id || getItemStackSize(selectedItem.item) <= selectedItem.item.count) {
            return;
         } else {
            selectedItem.item.favorite = (selectedItem.item.favorite || trashedItem.favorite);
            selectedItem.item.count += 1;
         }

         trashedItem.count -= 1;
         if (trashedItem.count <= 0) {
            trashedItem = Item{};
         }

         playSound("click");
         anySelected = true;
         return;
      }
   }
}

// Helper functions

void Inventory::toggleInventoryOpen() {
   if (IsKeyReleased(toggleInventoryKey)) {
      playSound("click");
      open = !open;
   }
}

void Inventory::switchOnKeyPress(int key, int hotbarX) {
   if (IsKeyPressed(key)) {
      selectedX = hotbarX;
      selectedY = 0;
   }
}

void Inventory::switchOnMouseWheel() {
   float wheel = GetMouseWheelMove();
   if (wheel >= 1.0f) {
      selectedX = (selectedX + 1) % 10;
   } else if (wheel <= -1.0f) {
      selectedX -= 1;
      if (selectedX < 0) selectedX = 9;
   }
}

void Inventory::handleDiscarding() {
   // Pressed outside of the inventory
   if (anySelected && open && isMousePressedOutsideUI(MOUSE_BUTTON_LEFT)) {
      playSound("click");

      if (!selectedItem.item.favorite) {
         dropSelectedItem();
         return;
      }
   }

   if (anySelected && !open) {
      discardItem();
   }
}

void Inventory::discardItem() {
   if (selectedItem.address == &trashedItem) {
      if (selectedItem.fullSelect || trashedItem.id == 0) {
         trashedItem = selectedItem.item;
      } else if (addItemCount(trashedItem, selectedItem.item) && !placeItem(selectedItem.item)) {
         dropSelectedItem();
         return;
      }
   } else if (!selectedItem.fullSelect && !placeItem(selectedItem.item)) {
      dropSelectedItem();
      return;
   }

   anySelected = false;
   selectedItem.reset();
}

// Frame functions

Vector2 Inventory::getFramePosition(float x, float y, bool isSelected) {
   Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {x, y}), itemframeTopLeft);

   if (isSelected) {
      return Vector2Subtract(position, selectedItemFrameOffset);
   }
   return position;
}

Vector2 Inventory::getFrameSize(bool isSelected) {
   return (isSelected ? selectedItemFrameSize : itemframeSize);
}

bool Inventory::mouseOnFrame(const Vector2 &position, const Vector2 &size) {
   Rectangle bounds {position.x, position.y, size.x, size.y};
   return CheckCollisionPointRec(GetMousePosition(), bounds);
}

Texture& Inventory::getFrameTexture(bool isSelected, bool isFavorite) {
   if (isSelected && isFavorite) {
      return getTexture("small_frame_favorite_selected");
   } else if (isSelected) {
      return getTexture("small_frame_selected");
   } else if (isFavorite) {
      return getTexture("small_frame_favorite");
   } else {
      return getTexture("small_frame");
   }
}

Texture& Inventory::getTrashTexture(bool trashOccupied) {
   return getTexture(trashOccupied ? "small_frame" : "small_frame_trash");
}

// Item functions

void Inventory::dropSelectedItem() {
   Vector2 playerCenter = player.getCenter();
   playerCenter.x += (player.flipX ? 3 : -3);
   playerCenter.x = clamp<int>(playerCenter.x, 0, map.sizeX - 1);

   DroppedItem droppedItem (selectedItem.item, playerCenter.x, playerCenter.y);
   droppedItems.push_back(droppedItem);

   if (selectedItem.fullSelect) {
      *selectedItem.address = Item{};
   }
   selectedItem.reset();
   anySelected = false;
}

bool Inventory::placeItem(Item &item) {
   Item *firstAvailableSpot = nullptr;
   
   // Check for equal items
   for (int y = 0; y < inventoryHeight; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item &it = items[y][x];

         if (it.id == item.id && it.isFurniture == item.isFurniture && addItemCount(it, item) <= 0) {
            return true;
         } else if (!firstAvailableSpot && it.id == 0) {
            firstAvailableSpot = &it;
         }
      }
   }

   // Place at the first available spot
   if (firstAvailableSpot) {
      *firstAvailableSpot = item;
      return true;
   }
   return false;
}

int Inventory::getItemStackSize(const Item &item) {
   if (item.type == Item::item) {
      return itemStackSize;
   } else if (item.type == Item::equipment) {
      return equipmentStackSize;
   } else {
      return potionStackSize;
   }
}

int Inventory::addItemCount(Item &item1, Item &item2) {
   int maximum = getItemStackSize(item1);
   int total = item1.count + item2.count;
   int last = item1.count;

   item1.count = min(total, maximum);
   int leftover = total - maximum;

   item2.count -= item1.count - last;
   if (item2.count <= 0) {
      item2 = Item{};
   }
   return leftover;
}

// Render functions

void Inventory::render() {
   for (int y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item &item = items[y][x];
         bool isSelected = (x == selectedX && y == selectedY);
         bool isFavorite = item.favorite;

         Vector2 position = getFramePosition(x, y, isSelected);
         Vector2 size = getFrameSize(isSelected);

         drawTextureNoOrigin(getFrameTexture(isSelected, isFavorite), position, size);
         if (item.id != 0 && (!anySelected || !selectedItem.fullSelect || selectedItem.address != &item)) {
            renderItem(item, position, false);
         }

         if (y == 0) {
            Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
            drawText(textPosition, std::to_string(x + 1).c_str(), 25);
         }
      }
   }

   // Render trash frame if inventory is open
   if (open) {
      bool trashOccupied = (trashedItem.id != 0 && (!anySelected || !selectedItem.fullSelect || selectedItem.address != &trashedItem));
      Vector2 position = getFramePosition(inventoryWidth - 1, inventoryHeight, false);
      Vector2 size = getFrameSize(false);

      drawTextureNoOrigin(getTrashTexture(trashOccupied), position, size);
      if (trashOccupied) {
         renderItem(trashedItem, position, false);
      }
   }

   // Render selected item
   if (anySelected) {
      renderItem(selectedItem.item, GetMousePosition(), true);
   }
}

void Inventory::renderItem(Item &item, const Vector2 &position, bool isSelected) {
   Color drawColor = (isSelected ? Fade(WHITE, 0.75f) : WHITE);

   if (!item.isFurniture) {
      drawTextureNoOrigin(getTexture(Block::getName(item.id)), Vector2Add(position, itemframeItemOffset), itemframeItemSize, drawColor);
   } else if (item.isFurniture) {
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

   if (item.count != 1) {
      Vector2 textPosition = Vector2Subtract(Vector2Add(position, itemframeSize), itemframeIndexOffset);
      drawText(textPosition, std::to_string(item.count).c_str(), 25, drawColor);
   }
}
