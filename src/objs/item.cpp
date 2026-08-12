#include "objs/item.hpp"
#include "SRU/util.hpp"
#include "SRU/render.hpp"
#include "objs/map.hpp"
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <unordered_map>

// constants

constexpr int droppedItemLifetime      = 60.0f * 15.0f;
constexpr float droppedItemFloatSpeed  = 1.5f;
constexpr float droppedItemFloatHeight = 0.25f;
constexpr Vector2 droppedItemSize      = {0.8f, 0.8f};

static const std::unordered_map<std::string, ItemActionType> itemActionTypeStrings {{
   {"none", ItemActionType::none}, {"place_block", ItemActionType::placeBlock}, {"place_wall", ItemActionType::placeWall},
   {"place_furniture", ItemActionType::placeFurniture}, {"place_liquid", ItemActionType::placeLiquid}
}};

static size_t itemCount = 1; // 0 - nil
static std::vector<std::string> itemNames {""};
static std::vector<ItemData> itemData {{}};
static std::unordered_map<std::string, itemid_t> itemIds;

// item getter functions

bool isItemActionValid(const std::string &action) {
   return itemActionTypeStrings.find(action) != itemActionTypeStrings.end();
}

bool isItemNameValid(const std::string &name) {
   return itemIds.find(name) != itemIds.end();
}

bool isItemIdValid(itemid_t id) {
   return id >= 0 && id < itemCount;
}

ItemActionType getItemActionTypeFromString(const std::string &action) {
   if (auto it = itemActionTypeStrings.find(action); it != itemActionTypeStrings.end()) {
      return it->second;
   }
   return ItemActionType::none;
}

ItemData &getItemData(itemid_t id) {
   return itemData[id];
}

itemid_t getItemIdFromName(const std::string &name) {
   return itemIds.at(name);
}

std::string getItemNameFromId(itemid_t id) {
   return itemNames[id];
}

size_t getItemCount() {
   return itemCount;
}

Texture getItemTexture(itemid_t id) {
   return itemData[id].texture;
}

Vector2 getItemSize(itemid_t id, Vector2 size) {
   Texture &texture = itemData[id].texture;
   float ratioX = fminf(1.0f, (float)texture.width / texture.height);
   float ratioY = fminf(1.0f, (float)texture.height / texture.width);
   return size * V2(ratioX, ratioY);
}

void reserveItemContainers(size_t estimate) {
   itemNames.reserve(estimate + 1);
   itemData.reserve(estimate + 1);
   itemIds.reserve(estimate + 1);
}

void pushItem(const std::string &name) {
   itemData.push_back({});
   itemNames.push_back(name);
   itemIds[name] = itemCount;
   itemCount += 1;
}

void setItem(const std::string &name, ItemData data) {
   itemid_t id = itemIds[name];
   itemData[id] = data;
}

// Dropped item functions

DroppedItem::DroppedItem(int count, itemid_t id, float lifetime, int tileX, int tileY)
   : count(count), id(id), lifetime(lifetime), tileX(tileX), tileY(tileY) {}

DroppedItem::DroppedItem(Item &item, int tileX, int tileY)
   : count(item.count), id(item.id), lifetime(0.0f), tileX(tileX), tileY(tileY) {}

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
   Vector2 position = V2(tileX, tileY - offsetY);
   Vector2 size = getItemSize(id, droppedItemSize);
   Color tint = (itemData[id].action == ItemActionType::placeWall && itemData[id].wall != 0 ? wallTint : WHITE);
   drawTexture(getItemTexture(id), position, size, tint);

   if (count > 1) {
      Vector2 textPosition = position + V2(0.0f, 0.7f);
      drawTextCentered("andy", textPosition, TextFormat("%d", count), 0.75f);
   }
}

Rectangle DroppedItem::getBounds() const {
   return {(float)tileX, (float)tileY, 1.0f, 1.0f};
}
