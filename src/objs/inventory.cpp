#include "mngr/input.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "util/math.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <array>

// Constants

constexpr Vector2 itemframeSize           = {60.0f, 60.0f};
constexpr Vector2 itemframePadding        = {itemframeSize.x + 5.0f, itemframeSize.y + 5.0f};
constexpr Vector2 itemframeTopLeft        = {15.0f, 15.0f};
constexpr Vector2 itemframeIndexOffset    = {15.0f, 15.0f};
constexpr Vector2 itemFrameCountOffset    = {0.0f, -itemframeSize.y / 2.0f + 10.0f};
constexpr Vector2 itemframeItemSize       = {30.0f, 30.0f};
constexpr Vector2 selectedItemFrameSize   = {65.0f, 65.0f};
constexpr Vector2 selectedItemFrameOffset = {(selectedItemFrameSize.x - itemframeSize.x) / 2.0f, (selectedItemFrameSize.y - itemframeSize.y) / 2.0f};

constexpr int itemStackSize      = 9999;
constexpr int equipmentStackSize = 1;
constexpr int potionStackSize    = 99;

constexpr size_t itemCount = 4;
constexpr static inline std::array<const char*, itemCount> itemTextures {{
   "coal_lump",
   "iron_lump",
   "gold_lump",
   "mythril_lump",
}};

constexpr size_t toolCount = 5;
struct ToolInfo {
   const char *texture;
   float breakMultiplier;
   int breakLevel;
};

constexpr static inline std::array<ToolInfo, toolCount> toolInfo {{
   ToolInfo{"wooden_pickaxe",  1.3f, 1},
   ToolInfo{"stone_pickaxe",   1.6f, 2},
   ToolInfo{"iron_pickaxe",    2.0f, 3},
   ToolInfo{"gold_pickaxe",    2.2f, 3},
   ToolInfo{"mythril_pickaxe", 2.5f, 3},
}};

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

            if (&item == selectedItem.address) {
               discardItem();
               return;
            }

            if (selectedItem.fullSelect) {
               if (item.id == selectedItem.item.id) {
                  addItemCount(item, *selectedItem.address);
                  item.favorite = (item.favorite || selectedItem.item.favorite);
               } else if (selectedItem.fromTrash && item.favorite) {
                  discardItem();
                  return;
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
            resetMousePress(MOUSE_BUTTON_LEFT);
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

      resetMousePress(MOUSE_BUTTON_LEFT);
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

   // Discard the first row of items only if a hotbar item
   // has been selected
   if (!open && anySelected && 10 > (reinterpret_cast<unsigned long long>(selectedItem.address) - reinterpret_cast<unsigned long long>(items)) / sizeof(Item)) {
      discardItem();
   }
}

// Placement functions

void Inventory::tryToPlaceItemOrDropAtCoordinates(Item &item, int x, int y) {
   if (!placeItem(item)) {
      DroppedItem droppedItem {item, x, y};
      droppedItems.push_back(droppedItem);
   }
}

bool Inventory::canPlaceBlock() {
   const Item &selected = getSelected();
   return selected.type == ItemType::block && selected.count > 0 && selected.id != 0;
}

void Inventory::placeBlock(int x, int y, bool playerFacingLeft) {
   Item &item = (anySelected ? selectedItem.item : items[selectedY][selectedX]);

   if (item.isFurniture) {
      Furniture furniture = getFurniture(x, y, map, getFurnitureType(item.id), playerFacingLeft);
      if (furniture.type == FurnitureType::none) {
         return;
      }
      map.addFurniture(furniture);
      player.placedBlock = true;
   } else {
      if (!((item.isWall && (map.walls[y][x].type & BlockType::empty)) || (!item.isWall && (map.blocks[y][x].type & BlockType::empty) && !(map.blocks[y][x].type & BlockType::furniture)))) {
         return;
      }
      map.setBlock(x, y, item.id, item.isWall);
      player.placedBlock = true;
   }

   item.count -= 1;
   if (anySelected && selectedItem.fullSelect) {
      selectedItem.address->count -= 1;
   }

   if (item.count <= 0) {
      if (anySelected) {
         if (selectedItem.fullSelect) {
            *selectedItem.address = Item{};
         }
         selectedItem.reset();
         anySelected = false;
      } else {
         item = Item{};
      }
   }
}

void Inventory::selectItem(int x, int y) {
   unsigned short id = 0;
   bool furniture = false;
   bool wall = false;

   if (map.blocks[y][x].type & BlockType::furniture) {
      id = map.getFurnitureAtPosition(x, y).id;
      furniture = true;
   } else if (!map.isEmpty(x, y)) {
      id = map.blocks[y][x].id;
   } else if (!(map.walls[y][x].type & BlockType::empty)) {
      id = map.walls[y][x].id;
      wall = true;
   } else {
      // Do not pick any elements
      return;
   }

   for (int y = 0; y < inventoryHeight; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item &item = items[y][x];

         if (item.isFurniture == furniture && item.isWall == wall && item.id == id) {
            if (y == 0) {
               selectedX = x;
               selectedY = 0;
            } else {
               std::swap(item, items[selectedY][selectedX]);
            }
            return;
         }
      }
   }
}

const Item &Inventory::getSelected() const {
   return anySelected ? selectedItem.item : items[selectedY][selectedX];
}

// Helper functions

void Inventory::toggleInventoryOpen() {
   if (isKeyPressed(KEY_E)) {
      playSound(open ? "ui_open_inventory" : "ui_close_inventory");
      open = !open;
   }
}

void Inventory::switchOnKeyPress(int key, int hotbarX) {
   if (isKeyPressed(key)) {
      if (!open && anySelected && selectedItem.address != &items[selectedY][selectedX]) {
         discardItem();
      }
      selectedX = hotbarX;
      selectedY = 0;
   }
}

void Inventory::switchOnMouseWheel() {
   float wheel = GetMouseWheelMove();
   if (!open && anySelected && selectedItem.address != &items[selectedY][selectedX]) {
      if (wheel >= 1.0f) {
         selectedX = 0;
         discardItem();
      } else if (wheel <= -1.0f) {
         selectedX = inventoryWidth - 1;
         discardItem();
      }
      return;
   }

   if (wheel >= 1.0f) {
      selectedX = (selectedX + 1) % inventoryWidth;
   } else if (wheel <= -1.0f) {
      selectedX -= 1;
      if (selectedX < 0) selectedX = inventoryWidth - 1;
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

Vector2 Inventory::getFramePosition(float x, float y, bool isSelected) const {
   Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {x, y}), itemframeTopLeft);

   if (isSelected) {
      return Vector2Subtract(position, selectedItemFrameOffset);
   }
   return position;
}

Vector2 Inventory::getFrameSize(bool isSelected) const {
   return (isSelected ? selectedItemFrameSize : itemframeSize);
}

bool Inventory::mouseOnFrame(const Vector2 &position, const Vector2 &size) {
   Rectangle bounds {position.x, position.y, size.x, size.y};
   return CheckCollisionPointRec(GetMousePosition(), bounds);
}

const Texture& Inventory::getFrameTexture(bool isSelected, bool isFavorite) const {
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

const Texture& Inventory::getTrashTexture(bool trashOccupied) const {
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

         if (it.id == item.id && it.isWall == item.isWall && it.isFurniture == item.isFurniture && addItemCount(it, item) <= 0) {
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
   if (item.type == ItemType::item || item.type == ItemType::block) {
      return itemStackSize;
   } else if (item.type == ItemType::equipment) {
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

int Inventory::getBlockBreakingLevel() {
   if (anySelected && selectedItem.item.type == ItemType::equipment && selectedItem.item.id <= toolCount) {
      return toolInfo.at(selectedItem.item.id - 1).breakLevel;
   } else if (items[selectedY][selectedX].type == ItemType::equipment && items[selectedY][selectedX].id <= toolCount) {
      return toolInfo.at(items[selectedY][selectedX].id - 1).breakLevel;
   }
   return 0;
}

float Inventory::getBlockBreakingMultiplier() {
   if (anySelected && selectedItem.item.type == ItemType::equipment && selectedItem.item.id <= toolCount) {
      return toolInfo.at(selectedItem.item.id - 1).breakMultiplier;
   } else if (items[selectedY][selectedX].type == ItemType::equipment && items[selectedY][selectedX].id <= toolCount) {
      return toolInfo.at(items[selectedY][selectedX].id - 1).breakMultiplier;
   }
   return 1.0f;
}

Texture2D *Inventory::getCurrentToolsTexture() const {
   if (anySelected && selectedItem.item.type == ItemType::equipment && selectedItem.item.id <= toolCount) {
      return &getTexture(toolInfo.at(selectedItem.item.id - 1).texture);
   } else if (items[selectedY][selectedX].type == ItemType::equipment && items[selectedY][selectedX].id <= toolCount) {
      return &getTexture(toolInfo.at(items[selectedY][selectedX].id - 1).texture);
   }
   return nullptr;
}

// Render functions

void Inventory::render() const {
   bool externalSlot = !open && anySelected && selectedItem.address != &items[selectedY][selectedX];

   for (int y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         const Item &item = items[y][x];
         bool isSelected = (x == selectedX && y == selectedY && !externalSlot);
         bool isFavorite = item.favorite;

         Vector2 position = getFramePosition(x, y, isSelected);
         Vector2 size = getFrameSize(isSelected);

         drawTextureNoOrigin(getFrameTexture(isSelected, isFavorite), position, size);
         if (item.id != 0 && (!anySelected || !selectedItem.fullSelect || selectedItem.address != &item)) {
            drawItem(item.type, item.id, item.count, item.isFurniture, item.isWall, Vector2Add(position, getOrigin(size)), itemframeItemSize, false);
         }

         if (y == 0) {
            Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
            drawText(textPosition, std::to_string(x + 1).c_str(), 25);
         }
      }
   }

   // Render external slot
   if (externalSlot) {
      Vector2 position = getFramePosition(10, 0, true);
      Vector2 size = getFrameSize(true);
      drawTextureNoOrigin(getFrameTexture(true, selectedItem.address->favorite),position, size);
      drawItem(selectedItem.item.type, selectedItem.item.id, selectedItem.item.count, selectedItem.item.isFurniture, selectedItem.item.isWall, Vector2Add(position, getOrigin(size)), itemframeItemSize, true);

      Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
      if (selectedItem.fromTrash) {
         drawText(textPosition, "BIN", 25);
      } else {
         // Pointer arithmetic
         int index = (reinterpret_cast<unsigned long long>(selectedItem.address) - reinterpret_cast<unsigned long long>(items)) / sizeof(Item);
         drawText(textPosition, std::to_string(index + 1).c_str(), 25);
      }
   }

   // Render trash frame if inventory is open
   if (open) {
      bool trashOccupied = (trashedItem.id != 0 && (!anySelected || !selectedItem.fullSelect || selectedItem.address != &trashedItem));
      Vector2 position = getFramePosition(inventoryWidth - 1, inventoryHeight, false);
      Vector2 size = getFrameSize(false);

      drawTextureNoOrigin(getTrashTexture(trashOccupied), position, size);
      if (trashOccupied) {
         drawItem(trashedItem.type, trashedItem.id, trashedItem.count, trashedItem.isFurniture, trashedItem.isWall, Vector2Add(position, getOrigin(size)), itemframeItemSize, false);
      }
   }

   // Render selected item
   if (anySelected && !externalSlot) {
      drawItem(selectedItem.item.type, selectedItem.item.id, selectedItem.item.count, selectedItem.item.isFurniture, selectedItem.item.isWall, GetMousePosition(), itemframeItemSize, true);
   }
}

// Draw item

void drawItem(ItemType type, unsigned short id, unsigned short count, bool isFurniture, bool isWall, const Vector2 &position, const Vector2 &size, bool isSelected, bool isworldspace) {
   if (type == ItemType::item && id <= itemCount) {
      Texture2D &texture = getTexture(itemTextures.at(id - 1));
      drawTexture(texture, position, size);

      // fuck dry
      if (count != 1) {
         Color textColor = (isSelected ? Fade(WHITE, 0.75f) : WHITE);
         Vector2 textPosition = Vector2Subtract(position, (isworldspace ? Vector2{0.0f, -0.7f} : itemFrameCountOffset));
         drawText(textPosition, std::to_string(count).c_str(), (isworldspace ? 0.75f : 25.0f), textColor, (isworldspace ? 0.1f : 1.0f));
      }
      return;
   }
   
   if (type == ItemType::equipment && id <= toolCount) {
      Texture2D &texture = getTexture(toolInfo[id - 1].texture);
      drawTexture(texture, position, size);
      return;
   }
   
   Color drawColor = Fade((isWall ? wallTint : WHITE), (isSelected ? 0.75f : 1.0f));

   if (!isFurniture) {
      Texture2D &texture = getTexture(getBlockNameFromId(id));
      DrawTexturePro(texture, {0, 0, 8, 8}, {position.x, position.y, size.x, size.y}, getOrigin(size), 0, drawColor);
   } else if (isFurniture) {
      FurnitureTexture texture = getFurnitureIcon(id);
      Vector2 newPos = position;//Vector2Add(position, Vector2Scale(itemframeSize, 0.5f));
      Vector2 fSize = size;

      if (texture.sizeX < texture.sizeY) {
         fSize.x *= texture.sizeX / texture.sizeY;
      } else if (texture.sizeX > texture.sizeY) {
         fSize.y *= texture.sizeY / texture.sizeX;
      }
      DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {newPos.x, newPos.y, fSize.x, fSize.y}, Vector2Scale(fSize, 0.5f), 0, drawColor);
   }

   if (count != 1) {
      Color textColor = (isSelected ? Fade(WHITE, 0.75f) : WHITE);
      Vector2 textPosition = Vector2Subtract(position, (isworldspace ? Vector2{0.0f, -0.7f} : itemFrameCountOffset));
      drawText(textPosition, std::to_string(count).c_str(), (isworldspace ? 0.75f : 25.0f), textColor, (isworldspace ? 0.1f : 1.0f));
   }
}
