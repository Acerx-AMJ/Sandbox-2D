#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "objs/player.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp" // IWYU pragma: export
#include "util/parallax.hpp"
#include "util/random.hpp"
#include <fstream>
#include <vector>

// Constants

constexpr int fileVersion = 1;

// File functions

std::string getRandomLineFromFile(const std::string &path) {
   return random(getAllLinesFromFile(path));
}

std::vector<std::string> getAllLinesFromFile(const std::string &path) {
   std::fstream file (path.c_str());
   if (!file.is_open()) {
      return {};
   }

   std::vector<std::string> lines;
   std::string line;

   while (std::getline(file, line)) {
      lines.push_back(line);
   }
   return lines;
}

void saveLinesToFile(const std::string &path, const std::vector<std::string> &lines) {
   std::ofstream file (path);
   assert(file.is_open(), "Failed to write to file '{}'.", path);

   for (const std::string &line: lines) {
      file << line << '\n';
   }
}

// World saving functions
// Save and load functions must follow the same data arrangement

static_assert(std::is_trivially_copyable_v<Item>);
static_assert(std::is_trivially_copyable_v<FurniturePiece>);
static_assert(std::is_trivially_copyable_v<DroppedItem>);

void saveWorldData(const std::string &name, float playerX, float playerY, float zoom, const Map &map, const Inventory *inventory, const std::vector<DroppedItem> *droppedItems) {
   std::ofstream file ("data/worlds/" + name + ".bin", std::ios::binary);
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.bin'.", name);

   // Write basic data
   file.write(reinterpret_cast<const char*>(&fileVersion), sizeof(fileVersion));
   file.write(reinterpret_cast<const char*>(&playerX), sizeof(playerX));
   file.write(reinterpret_cast<const char*>(&playerY), sizeof(playerY));
   file.write(reinterpret_cast<const char*>(&map.sizeX), sizeof(map.sizeX));
   file.write(reinterpret_cast<const char*>(&map.sizeY), sizeof(map.sizeY));
   file.write(reinterpret_cast<const char*>(&zoom), sizeof(zoom));

   int timeOfDay = (inventory ? getLastTimeOfDay() : 0);
   int moonPhase = (inventory ? getLastMoonPhase() : 0);
   int selectedX = (inventory ? inventory->selectedX : 0);
   int selectedY = (inventory ? inventory->selectedY : 0);

   file.write(reinterpret_cast<const char*>(&timeOfDay), sizeof(timeOfDay));
   file.write(reinterpret_cast<const char*>(&moonPhase), sizeof(moonPhase));
   file.write(reinterpret_cast<const char*>(&selectedX), sizeof(selectedX));
   file.write(reinterpret_cast<const char*>(&selectedY), sizeof(selectedY));

   // Write inventory
   if (inventory) {
      file.write(reinterpret_cast<const char*>(inventory->items[0]), inventoryHeight * inventoryWidth * sizeof(Item));
   } else {
      Item item;
      for (int i = 0; i < inventoryHeight * inventoryWidth; ++i) {
         file.write(reinterpret_cast<const char*>(&item), sizeof(Item));
      }
   }

   // Write the map
   for (const std::vector<Block> &row: map.blocks) {
      for (const Block &tile: row) {
         file.write(reinterpret_cast<const char*>(&tile.id), sizeof(tile.id));
         file.write(reinterpret_cast<const char*>(&tile.value2), sizeof(tile.value2));
      }
   }

   for (const std::vector<Block> &row: map.walls) {
      for (const Block &tile: row) {
         file.write(reinterpret_cast<const char*>(&tile.id), sizeof(tile.id));
      }
   }

   // Write the furniture
   size_t furnitureCount = map.furniture.size();
   file.write(reinterpret_cast<const char*>(&furnitureCount), sizeof(furnitureCount));

   for (const Furniture &obj: map.furniture) {
      file.write(reinterpret_cast<const char*>(&obj.posX), sizeof(obj.posX));
      file.write(reinterpret_cast<const char*>(&obj.posY), sizeof(obj.posY));
      file.write(reinterpret_cast<const char*>(&obj.sizeX), sizeof(obj.sizeX));
      file.write(reinterpret_cast<const char*>(&obj.sizeY), sizeof(obj.sizeY));
      file.write(reinterpret_cast<const char*>(&obj.type), sizeof(obj.type));
      file.write(reinterpret_cast<const char*>(&obj.value), sizeof(obj.value));
      file.write(reinterpret_cast<const char*>(&obj.value2), sizeof(obj.value2));
      file.write(reinterpret_cast<const char*>(&obj.texId), sizeof(obj.texId));

      for (const std::vector<FurniturePiece> &row: obj.pieces) {
         file.write(reinterpret_cast<const char*>(row.data()), row.size() * sizeof(FurniturePiece));
      }
   }

   // Write dropped items
   size_t droppedItemCount = (droppedItems ? droppedItems->size() : 0);
   file.write(reinterpret_cast<const char*>(&droppedItemCount), sizeof(droppedItemCount));
   if (droppedItems) {
      file.write(reinterpret_cast<const char*>(droppedItems->data()), droppedItems->size() * sizeof(DroppedItem));
   }
}

// World loading functions

void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map, Inventory &inventory, std::vector<DroppedItem> &droppedItems) {
   std::ifstream file ("data/worlds/" + name + ".bin", std::ios::binary);
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.bin'.", name);

   // Read basic data
   int versionOfFile = 0;
   file.read(reinterpret_cast<char*>(&versionOfFile), sizeof(versionOfFile));
   file.read(reinterpret_cast<char*>(&player.position.x), sizeof(player.position.x));
   file.read(reinterpret_cast<char*>(&player.position.y), sizeof(player.position.y));
   file.read(reinterpret_cast<char*>(&map.sizeX), sizeof(map.sizeX));
   file.read(reinterpret_cast<char*>(&map.sizeY), sizeof(map.sizeY));
   file.read(reinterpret_cast<char*>(&zoom), sizeof(zoom));

   int timeofDay = 0;
   int moonPhase = 0;
   file.read(reinterpret_cast<char*>(&timeofDay), sizeof(timeofDay));
   file.read(reinterpret_cast<char*>(&moonPhase), sizeof(moonPhase));
   getTimeOfDay() = timeofDay;
   getMoonPhase() = moonPhase;

   file.read(reinterpret_cast<char*>(&inventory.selectedX), sizeof(inventory.selectedX));
   file.read(reinterpret_cast<char*>(&inventory.selectedY), sizeof(inventory.selectedY));
   map.init();

   // Read inventory
   file.read(reinterpret_cast<char*>(&inventory.items[0]), inventoryHeight * inventoryWidth * sizeof(Item));

   // Read map
   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         unsigned char id = 0;
         unsigned char value2 = 0;
         file.read(reinterpret_cast<char*>(&id), sizeof(id));
         file.read(reinterpret_cast<char*>(&value2), sizeof(value2));
         map.setBlock(x, y, (unsigned char)id);
         map[y][x].value2 = value2;
      }
   }

   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         unsigned char id = 0;
         file.read(reinterpret_cast<char*>(&id), sizeof(id));
         map.setBlock(x, y, (unsigned char)id, true);
      }
   }

   // Read furniture
   size_t furnitureCount = 0;
   file.read(reinterpret_cast<char*>(&furnitureCount), sizeof(furnitureCount));

   for (size_t i = 0; i < furnitureCount; ++i) {
      Furniture obj;
      file.read(reinterpret_cast<char*>(&obj.posX), sizeof(obj.posX));
      file.read(reinterpret_cast<char*>(&obj.posY), sizeof(obj.posY));
      file.read(reinterpret_cast<char*>(&obj.sizeX), sizeof(obj.sizeX));
      file.read(reinterpret_cast<char*>(&obj.sizeY), sizeof(obj.sizeY));
      file.read(reinterpret_cast<char*>(&obj.type), sizeof(obj.type));
      file.read(reinterpret_cast<char*>(&obj.value), sizeof(obj.value));
      file.read(reinterpret_cast<char*>(&obj.value2), sizeof(obj.value2));
      file.read(reinterpret_cast<char*>(&obj.texId), sizeof(obj.texId));

      obj.pieces.resize(obj.sizeY, std::vector<FurniturePiece>(obj.sizeX));
      for (std::vector<FurniturePiece> &row: obj.pieces) {
         file.read(reinterpret_cast<char*>(row.data()), row.size() * sizeof(FurniturePiece));
      }
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
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.bin'.", name);

   int versionOfFile = 0;
   file.read(reinterpret_cast<char*>(&versionOfFile), sizeof(versionOfFile));
   return versionOfFile;
}

int getLatestVersion() {
   return fileVersion;
}
