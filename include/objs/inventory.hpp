#pragma once
#include "objs/item.hpp"
#include <vector>

constexpr inline int inventoryWidth  = 10;
constexpr inline int inventoryHeight = 4;
constexpr inline int inventorySlots = inventoryWidth * inventoryHeight + 1; // trash included
constexpr inline int trashSlot = inventorySlots - 1;

struct Inventory {
   // update
   
   void update(bool canSwitchOnScroll);
   void toggleInventoryOpen();
   void switchOnKeyPress(int key, int hotbarX);
   void switchOnMouseWheel();

   // item functions

   void discardSelection();
   void placeItemOrDrop(Item &item);
   void dropItem(Item &item);
   bool placeItem(Item &item);
   int addItemCount(Item &item1, Item &item2);
   void pickItem(blockid_t blockId, blockid_t wallId, furnitureid_t furnitureId, liquidid_t liquidId);

   bool anyItemSelected();
   ItemData &getSelectedItem();
   void useSelectedItem();

   // frame functions

   Texture getFrameTexture(int i);
   Color getItemColor(Item item);

   // render
   
   void renderFrame(Font font, float fontSize, Vector2 position, Vector2 size, Item item, int i, bool externalSlot);
   void render();

   // members

   std::vector<Item> pendingDrops;
   Item items[inventorySlots];
   SelectedItem selection;
   int selected = 0;
   bool open = false;
};
