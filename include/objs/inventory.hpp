#ifndef OBJS_INVENTORY_HPP
#define OBJS_INVENTORY_HPP

#include "objs/item.hpp"
#include "objs/player.hpp"
#include <vector>

// Constants

constexpr inline int inventoryWidth  = 10;
constexpr inline int inventoryHeight = 4;

// Inventory

struct Inventory {
   Map &map;
   Player &player;
   std::vector<DroppedItem> &droppedItems;

   Item items[inventoryHeight][inventoryWidth];
   int selectedX = 0, selectedY = 0;
   bool open = false;

   bool anySelected = false;
   SelectedItem selectedItem;
   Item trashedItem;

   // Constructors

   Inventory(Map &map, Player &player, std::vector<DroppedItem> &droppedItems);

   // Update functions

   void update();

   // Helper functions

   void toggleInventoryOpen();
   void switchOnKeyPress(int key, int hotbarX);
   void switchOnMouseWheel();

   void handleDiscarding();
   void discardItem();

   // Frame functions

   Vector2 getFramePosition(float x, float y, bool isSelected) const;
   Vector2 getFrameSize(bool isSelected) const;
   bool mouseOnFrame(const Vector2 &position, const Vector2 &size);

   const Texture& getFrameTexture(bool isSelected, bool isFavorite) const;
   const Texture& getTrashTexture(bool trashOccupied) const;

   // Item functions
   
   void dropSelectedItem();
   bool placeItem(Item &item);
   int getItemStackSize(const Item &item);
   int addItemCount(Item &item1, Item &item2);

   // Render functions
   
   void render() const;
   void renderItem(const Item &item, const Vector2 &position, bool isSelected) const;
};

#endif
