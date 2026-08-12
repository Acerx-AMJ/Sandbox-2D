#include "mngr/fileio.hpp"
#include "objs/console.hpp"
#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "objs/parallax.hpp"
#include "objs/player.hpp"
#include <fstream>
#include <vector>

// Please increment after any breaking changes to warn players
// about corrupted worlds
constexpr int fileVersion = 11;

// World saving functions
// Save and load functions must follow the same data arrangement

void saveWorldData(const std::string &name, const Vector2 &playerSpawnPosition, const Vector2 &position, bool creative, int breath, int hearts, int maxHearts, float zoom, const Map &map, const Console *console, const Inventory *inventory, const std::vector<DroppedItem> *droppedItems) {
   std::ofstream file ("data/worlds/" + name + ".bin", std::ios::binary);
   if (!file.is_open()) {
      printf("saveWorldData: Failed to save world 'data/worlds/%s.bin'.\n", name.c_str());
      return;
   }

   // Write basic data
   file.write(reinterpret_cast<const char*>(&fileVersion), sizeof(fileVersion));
   file.write(reinterpret_cast<const char*>(&playerSpawnPosition.x), sizeof(playerSpawnPosition.x));
   file.write(reinterpret_cast<const char*>(&playerSpawnPosition.y), sizeof(playerSpawnPosition.y));
   file.write(reinterpret_cast<const char*>(&position.x), sizeof(position.x));
   file.write(reinterpret_cast<const char*>(&position.y), sizeof(position.y));
   file.write(reinterpret_cast<const char*>(&creative), sizeof(creative));
   file.write(reinterpret_cast<const char*>(&breath), sizeof(breath));
   file.write(reinterpret_cast<const char*>(&hearts), sizeof(hearts));
   file.write(reinterpret_cast<const char*>(&maxHearts), sizeof(maxHearts));
   file.write(reinterpret_cast<const char*>(&map.sizeX), sizeof(map.sizeX));
   file.write(reinterpret_cast<const char*>(&map.sizeY), sizeof(map.sizeY));
   file.write(reinterpret_cast<const char*>(&zoom), sizeof(zoom));

   float timeOfDay = (inventory ? getTimeOfDay() : 0);
   int moonPhase = (inventory ? getMoonPhase() : 0);

   file.write(reinterpret_cast<const char*>(&timeOfDay), sizeof(timeOfDay));
   file.write(reinterpret_cast<const char*>(&moonPhase), sizeof(moonPhase));

   // Write inventory
   if (inventory) {
      file.write(reinterpret_cast<const char*>(inventory->items), inventorySlots * sizeof(Item));
   } else {
      Item item;
      for (int i = 0; i < inventorySlots; ++i) {
         file.write(reinterpret_cast<const char*>(&item), sizeof(Item));
      }
   }

   // Write console history
   if (console) {
      size_t size = console->history.size();
      file.write(reinterpret_cast<const char*>(&size), sizeof(size));
      for (const std::string &string : console->history) {
         size = string.size();
         file.write(reinterpret_cast<const char*>(&size), sizeof(size));
         file.write(reinterpret_cast<const char*>(string.data()), string.size());
      }
   } else {
      size_t none = 0;
      file.write(reinterpret_cast<const char*>(&none), sizeof(none));
   }

   // Write the map
   int blockCount = map.sizeX * map.sizeY;
   std::vector<blockid_t> blocks, walls;
   blocks.reserve(blockCount);
   walls.reserve(blockCount);

   for (const Block &tile: map.blocks) {
      blocks.push_back(tile.id);
   }

   for (const Wall &tile: map.walls) {
      walls.push_back(tile.id);
   }

   file.write(reinterpret_cast<const char*>(blocks.data()), blocks.size() * sizeof(blockid_t));
   file.write(reinterpret_cast<const char*>(walls.data()), walls.size() * sizeof(blockid_t));
   file.write(reinterpret_cast<const char*>(map.liquidHeights.data()), map.liquidHeights.size() * sizeof(liquidlayer_t));
   file.write(reinterpret_cast<const char*>(map.liquidTypes.data()), map.liquidTypes.size() * sizeof(liquidid_t));

   // Write the furniture
   size_t furnitureCount = map.furniture.size();
   file.write(reinterpret_cast<const char*>(&furnitureCount), sizeof(furnitureCount));

   for (const Furniture &obj: map.furniture) {
      file.write(reinterpret_cast<const char*>(&obj.id), sizeof(obj.id));
      file.write(reinterpret_cast<const char*>(&obj.x), sizeof(obj.x));
      file.write(reinterpret_cast<const char*>(&obj.y), sizeof(obj.y));
      file.write(reinterpret_cast<const char*>(&obj.width), sizeof(obj.width));
      file.write(reinterpret_cast<const char*>(&obj.height), sizeof(obj.height));
      file.write(reinterpret_cast<const char*>(&obj.ivalue1), sizeof(obj.ivalue1));
      file.write(reinterpret_cast<const char*>(&obj.ivalue2), sizeof(obj.ivalue2));
      file.write(reinterpret_cast<const char*>(&obj.fvalue1), sizeof(obj.fvalue1));
      file.write(reinterpret_cast<const char*>(&obj.fvalue2), sizeof(obj.fvalue2));
      file.write(reinterpret_cast<const char*>(obj.pieces.data()), obj.pieces.size() * sizeof(FurniturePiece));
   }

   // Write dropped items
   size_t droppedItemCount = (droppedItems ? droppedItems->size() : 0);
   file.write(reinterpret_cast<const char*>(&droppedItemCount), sizeof(droppedItemCount));
   if (droppedItems) {
      file.write(reinterpret_cast<const char*>(droppedItems->data()), droppedItems->size() * sizeof(DroppedItem));
   }
}

// World loading functions

void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map, Console &console, Inventory &inventory, std::vector<DroppedItem> &droppedItems) {
   std::ifstream file ("data/worlds/" + name + ".bin", std::ios::binary);
   if (!file.is_open()) {
      printf("loadWorldData: Failed to load world 'data/worlds/%s.bin'.\n", name.c_str());
      return;
   }

   // Read basic data
   int versionOfFile = 0;
   file.read(reinterpret_cast<char*>(&versionOfFile), sizeof(versionOfFile));
   file.read(reinterpret_cast<char*>(&player.spawnPos.x), sizeof(player.spawnPos.x));
   file.read(reinterpret_cast<char*>(&player.spawnPos.y), sizeof(player.spawnPos.y));
   file.read(reinterpret_cast<char*>(&player.position.x), sizeof(player.position.x));
   file.read(reinterpret_cast<char*>(&player.position.y), sizeof(player.position.y));
   file.read(reinterpret_cast<char*>(&player.creative), sizeof(player.creative));
   file.read(reinterpret_cast<char*>(&player.breath), sizeof(player.breath));
   file.read(reinterpret_cast<char*>(&player.hearts), sizeof(player.hearts));
   file.read(reinterpret_cast<char*>(&player.maxHearts), sizeof(player.maxHearts));
   file.read(reinterpret_cast<char*>(&map.sizeX), sizeof(map.sizeX));
   file.read(reinterpret_cast<char*>(&map.sizeY), sizeof(map.sizeY));
   file.read(reinterpret_cast<char*>(&zoom), sizeof(zoom));

   float timeofDay = 0;
   int moonPhase = 0;
   file.read(reinterpret_cast<char*>(&timeofDay), sizeof(timeofDay));
   file.read(reinterpret_cast<char*>(&moonPhase), sizeof(moonPhase));
   setTimeOfDay(timeofDay);
   setMoonPhase(moonPhase);
   map.init();

   // Read inventory
   file.read(reinterpret_cast<char*>(&inventory.items), inventorySlots * sizeof(Item));

   // Read console
   size_t size = 0;
   file.read(reinterpret_cast<char*>(&size), sizeof(size));
   console.history.resize(size);

   for (size_t i = 0; i < size; ++i) {
      size_t ssize = 0;
      file.read(reinterpret_cast<char*>(&ssize), sizeof(ssize));
      
      std::string history;
      history.resize(ssize);
      file.read(reinterpret_cast<char*>(history.data()), ssize);
      console.history[i] = history;
   }

   // Read map
   int blockCount = map.sizeX * map.sizeY;
   std::vector<blockid_t> blocks, walls;
   blocks.resize(blockCount);
   walls.resize(blockCount);

   file.read(reinterpret_cast<char*>(blocks.data()), blocks.size() * sizeof(blockid_t));
   file.read(reinterpret_cast<char*>(walls.data()), walls.size() * sizeof(blockid_t));
   file.read(reinterpret_cast<char*>(map.liquidHeights.data()), map.liquidHeights.size() * sizeof(liquidlayer_t));
   file.read(reinterpret_cast<char*>(map.liquidTypes.data()), map.liquidTypes.size() * sizeof(liquidid_t));

   for (int y = 0; y < map.sizeY; ++y) {
      map.setRow(y, blocks.data() + y * map.sizeX);
   }

   for (int y = 0; y < map.sizeY; ++y) {
      map.setWallRow(y, walls.data() + y * map.sizeX);
   }

   // Read furniture
   size_t furnitureCount = 0;
   file.read(reinterpret_cast<char*>(&furnitureCount), sizeof(furnitureCount));

   for (size_t i = 0; i < furnitureCount; ++i) {
      Furniture obj;
      file.read(reinterpret_cast<char*>(&obj.id), sizeof(obj.id));
      file.read(reinterpret_cast<char*>(&obj.x), sizeof(obj.x));
      file.read(reinterpret_cast<char*>(&obj.y), sizeof(obj.y));
      file.read(reinterpret_cast<char*>(&obj.width), sizeof(obj.width));
      file.read(reinterpret_cast<char*>(&obj.height), sizeof(obj.height));
      file.read(reinterpret_cast<char*>(&obj.ivalue1), sizeof(obj.ivalue1));
      file.read(reinterpret_cast<char*>(&obj.ivalue2), sizeof(obj.ivalue2));
      file.read(reinterpret_cast<char*>(&obj.fvalue1), sizeof(obj.fvalue1));
      file.read(reinterpret_cast<char*>(&obj.fvalue2), sizeof(obj.fvalue2));

      obj.pieces.resize(obj.width * obj.height);
      file.read(reinterpret_cast<char*>(obj.pieces.data()), obj.pieces.size() * sizeof(FurniturePiece));
      map.addFurniture(obj);
   }

   size_t droppedItemCount = 0;
   file.read(reinterpret_cast<char*>(&droppedItemCount), sizeof(droppedItemCount));
   droppedItems.resize(droppedItemCount);

   if (droppedItemCount > 0) {
      file.read(reinterpret_cast<char*>(droppedItems.data()), droppedItemCount * sizeof(DroppedItem));
   }
   player.init();
}

int getFileVersion(const std::string &name) {
   std::ifstream file ("data/worlds/" + name + ".bin", std::ios::binary);
   if (!file.is_open()) {
      printf("getFileVersion: Failed to load world 'data/worlds/%s.bin'.\n", name.c_str());
      return 0;
   }

   int versionOfFile = 0;
   file.read(reinterpret_cast<char*>(&versionOfFile), sizeof(versionOfFile));
   return versionOfFile;
}

int getLatestVersion() {
   return fileVersion;
}
