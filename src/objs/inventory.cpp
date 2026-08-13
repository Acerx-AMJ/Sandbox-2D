#include "SRU/text.hpp"
#include "SRU/util.hpp"
#include "mngr/input.hpp"
#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/render.hpp"
#include <raymath.h>
#include <algorithm>

// Constants

constexpr Vector2 inventoryGridPosition = {0.01f, 0.01f};
constexpr Vector2 inventoryGridSize = {0.35f * (inventoryWidth / float(inventoryHeight + 1)), 0.35f};
constexpr float slotScale = 0.95f;
constexpr float itemScale = 0.5f;
constexpr float itemFontSize = 25.0f;

// update functions

void Inventory::update(bool canSwitchOnScroll) {
   int lastSelected = selected;

   toggleInventoryOpen();
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

   if (canSwitchOnScroll) {
      switchOnMouseWheel();
   }

   if (selected != lastSelected) {
      playSound("hover");
   }

   // Handle item operations
   Rectangle grid = mapCubicArea(inventoryGridPosition, inventoryGridSize);
   int columns = inventoryWidth;
   int rows = inventoryHeight + 1;

   Vector2 slotSize = getGridCellSize(grid, columns, rows) * slotScale;
   Vector2 mousePos = GetMousePosition();
   int amount = (open ? inventorySlots : inventoryWidth);

   for (int i = 0; i < amount; ++i) {
      if (!CheckCollisionPointRec(mousePos, R4(gridPosition(grid, columns, rows, i % inventoryWidth, i / inventoryWidth) - slotSize / 2.0f, slotSize))) {
         continue;
      }
      Item &item = items[i];

      setMouseOnUI(true);
      bool mousePressed = isMousePressedUI(MOUSE_BUTTON_LEFT);
      bool shouldResetClick = true;
      bool isTrash = (i == trashSlot);
      bool isValid = (open && item.id != 0);

      // select frames while inventory is closed
      if (mousePressed && i < inventoryWidth) {
         if (!open) playSound("click");
         selected = i;
         shouldResetClick = false;
      }

      // favoriting
      if (mousePressed && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && isValid && !isTrash) {
         playSound("click");
         item.favorite = !item.favorite;
      }
      // quick-trashing
      else if (mousePressed && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && isValid && !isTrash && !item.favorite) {
         playSound("trash");
         items[trashSlot] = item;
         item = {};
      }
      // shift-clicking from trash
      else if (mousePressed && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && open && isTrash) {
         placeItem(item);
         playSound("click");
      }
      // swap/discard items
      else if (mousePressed && open && selection.selected) {
         playSound(i == trashSlot ? "trash" : "click");
         if (i == selection.lastSelection) {
            discardSelection();
            break;
         }

         if (item.id == 0) {
            item = selection.item;
            selection = {};
         }
         else if (item.id == selection.item.id) {
            addItemCount(item, selection.item);
            if (selection.item.count <= 0) {
               selection = {};
            }
         }
         else if (i == trashSlot) {
            items[trashSlot] = selection.item;
            selection = {};
         }
         else if (items[selection.lastSelection].id == 0) {
            items[selection.lastSelection] = item;
            items[i] = selection.item;
            selection = {};
         }
      }
      // handle selection
      else if (mousePressed && !selection.selected && isValid) {
         playSound("click");
         selection.lastSelection = i;
         selection.selected = true;
         selection.item = item;
         item = {};
      }
      // handle right-click picking
      else if (isValid && isMousePressedUI(MOUSE_BUTTON_RIGHT)) {
         int amount = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? std::ceil(item.count / 2.0f) : 1);
         if (!selection.selected) {
            selection.lastSelection = i;
            selection.selected = true;
            selection.item = {amount, item.id, item.favorite};

            item.count -= amount;
            if (item.count <= 0) item = {};
         }
         else if (item.id == selection.item.id && getItemData(selection.item.id).stackSize > selection.item.count) {
            int free = std::min(amount, getItemData(selection.item.id).stackSize - selection.item.count);
            selection.item.count += free;

            item.count -= free;
            if (item.count <= 0) item = {};
         }
         playSound("click");
      }

      if (shouldResetClick) {
         resetMousePress(MOUSE_BUTTON_LEFT);
      }
   }

   if (!open && selection.selected && selection.lastSelection < inventoryWidth) {
      discardSelection();
   }

   // Pressed outside of the inventory
   if (selection.selected && open && isMousePressedOutsideUI(MOUSE_BUTTON_LEFT)) {
      playSound("click");

      if (!selection.item.favorite) {
         dropItem(selection.item);
         selection = {};
         return;
      }
   }
}

void Inventory::toggleInventoryOpen() {
   if (isKeyPressed(KEY_E)) {
      playSound(open ? "ui_open_inventory" : "ui_close_inventory");
      open = !open;
   }
}

void Inventory::switchOnKeyPress(int key, int hotbarX) {
   if (isKeyPressed(key)) {
      if (!open && selection.selected && selection.lastSelection != selected) {
         discardSelection();
      }
      selected = hotbarX;
   }
}

void Inventory::switchOnMouseWheel() {
   float wheel = GetMouseWheelMove();
   if (!open && selection.selected && selection.lastSelection != selected) {
      if (wheel >= 1.0f) {
         selected = 0;
         discardSelection();
      } else if (wheel <= -1.0f) {
         selected = inventoryWidth - 1;
         discardSelection();
      }
   }
   else {
      if (wheel >= 1.0f) {
         selected = (selected + 1) % inventoryWidth;
      } else if (wheel <= -1.0f) {
         selected = (selected <= 0 ? inventoryWidth -1 : selected - 1);
      }
   }
}

// item functions

void Inventory::discardSelection() {
   if (!selection.selected) return;
   Item &target = items[selection.lastSelection];

   if (target.id == 0) {
      target = selection.item;
   }
   else if (addItemCount(target, selection.item) > 0) {
      placeItemOrDrop(selection.item);
   }
   selection = {};
}

void Inventory::placeItemOrDrop(Item &item) {
   if (!placeItem(item)) {
      dropItem(item);
   }
}

void Inventory::dropItem(Item &item) {
   pendingDrops.push_back(item);
   item = {};
}

bool Inventory::placeItem(Item &item2) {
   int firstAvailableSpot = inventorySlots;
   for (int i = 0; i < inventorySlots - 1; ++i) {
      Item &item1 = items[i];

      if (item1.id == item2.id && addItemCount(item1, item2) <= 0) {
         return true;
      }
      else if (firstAvailableSpot == inventorySlots && item1.id == 0) {
         firstAvailableSpot = i;
      }
   }

   if (firstAvailableSpot != inventorySlots) {
      items[firstAvailableSpot] = item2;
      item2 = {};
      return true;
   }
   return false;
}

int Inventory::addItemCount(Item &item1, Item &item2) {
   int stackSize = getItemData(item1.id).stackSize;
   int total = item1.count + item2.count;
   int leftover = total - stackSize;
   int last = item1.count;

   item1.count = std::min(total, stackSize);
   item2.count -= item1.count - last;

   if (item2.count <= 0) {
      item2 = {};
   }
   return leftover;
}

void Inventory::pickItem(blockid_t blockId, blockid_t wallId, furnitureid_t furnitureId, liquidid_t liquidId) {
   itemid_t target = 0;

   for (itemid_t i = 1; i < getItemCount(); ++i) {
      ItemData &data = getItemData(i);
      if ((data.action == ItemActionType::placeBlock && data.block == blockId)
       || (data.action == ItemActionType::placeWall && data.wall == wallId)
       || (data.action == ItemActionType::placeFurniture && data.furniture == furnitureId)
       || (data.action == ItemActionType::placeLiquid && data.liquid == liquidId)) {
         target = i;
         break;
      }
   }

   if (target == 0) return;
   for (int i = 0; i < inventorySlots - 1; ++i) {
      if (items[i].id == target && i < inventoryWidth) {
         selected = i;
         break;
      }
      else if (items[i].id == target && i >= inventoryWidth) {
         std::swap(items[selected], items[i]);
         break;
      }
   }
}

// frame functions

Texture Inventory::getFrameTexture(int i) const {
   if (i == trashSlot) {
      return getTexture(items[trashSlot].id == 0 ? "small_frame_trash" : "small_frame");
   }
   return getTexture(i == selected && items[i].favorite ? "small_frame_favorite_selected" : (i == selected ? "small_frame_selected" : (items[i].favorite ? "small_frame_favorite" : "small_frame")));
}

Color Inventory::getItemColor(Item item) const {
   return (getItemData(item.id).action == ItemActionType::placeWall && getItemData(item.id).wall != 0 ? wallTint : WHITE);
}

// render

void Inventory::renderFrame(Font font, float fontSize, Vector2 position, Vector2 size, Item item, int i, bool externalSlot) {
   drawTextureCentered(getFrameTexture(i), position, size);

   if (item.id != 0) {
      drawTextureCentered(getItemTexture(item.id), position, getItemSize(item.id, size * itemScale), getItemColor(item));
   }

   if (item.id != 0 && item.count > 1) {
      Vector2 textPosition = {position.x, position.y + size.y / 3.0f};
      drawTextCentered(font, textPosition, TextFormat("%d", item.count), fontSize);
   }

   if (i < inventoryWidth || externalSlot) {
      Vector2 textPosition = position - size / 3.0f;
      drawTextCentered(font, textPosition, (i == trashSlot ? "BIN" : TextFormat("%d", i)), fontSize);
   }
}

void Inventory::render() {
   Rectangle grid = mapCubicArea(inventoryGridPosition, inventoryGridSize);
   int columns = inventoryWidth;
   int rows = inventoryHeight + 1;

   Font font = getFont("andy");
   float fontSize = getFontSizeScaled(itemFontSize);

   Vector2 size = getGridCellSize(grid, columns, rows) * slotScale;
   int amount = (open ? inventorySlots : inventoryWidth);

   for (int i = 0; i < amount; ++i) {
      Vector2 position = gridPosition(grid, columns, rows, i % inventoryWidth, i / inventoryWidth);
      renderFrame(font, fontSize, position, size, items[i], i, false);
   }

   bool externalSlot = !open && selection.selected && selection.lastSelection >= inventoryWidth;
   if (externalSlot) {
      Vector2 position = gridPosition(grid, columns, rows, inventoryWidth, 0);
      renderFrame(font, fontSize, position, size, selection.item, selection.lastSelection, true);
   }

   if (selection.selected && !externalSlot) {
      drawTextureCentered(getItemTexture(selection.item.id), GetMousePosition(), getItemSize(selection.item.id, size * itemScale), getItemColor(selection.item));
      if (selection.item.count > 1) {
         Vector2 textPosition = V2(GetMouseX(), GetMouseY() + size.y / 3.0f);
         drawTextCentered(font, textPosition, TextFormat("%d", selection.item.count), fontSize);
      }
   }
}
