#include "mngr/fileio.hpp"
#include "objs/console.hpp"
#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "objs/parallax.hpp"
#include "objs/player.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

// Please increment after any breaking changes to warn players about corrupted worlds
constexpr int fileVersion = 13;

// Save and load functions must follow the same data arrangement. save here takes in optional arguments since world generator
// does not have them
void saveWorldData(const std::string &name, const Vector2 &playerSpawnPosition, const Vector2 &position, bool creative, int breath, int hearts, int maxHearts, float zoom, const Map &map, const Console *console, const Inventory *inventory, const std::vector<DroppedItem> *droppedItems) {
   auto begin = std::chrono::steady_clock::now();
   std::string filename = "data/worlds/" + name + ".bin";
   std::ofstream file (filename, std::ios::binary);

   if (!file.is_open()) {
      printf("saveWorldData: Failed to save world 'data/worlds/%s.bin'.\n", name.c_str());
      return;
   }

   // Write basic data. we check for inventory here since freshly generated maps don't pass it. and there's no reason
   // to save the time and moon phase of the main menu.
   float timeOfDay = (inventory ? getTimeOfDay() : 0);
   int moonPhase = (inventory ? getMoonPhase() : 0);

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
   file.write(reinterpret_cast<const char*>(&timeOfDay), sizeof(timeOfDay));
   file.write(reinterpret_cast<const char*>(&moonPhase), sizeof(moonPhase));

   // Write inventory
   if (inventory) {
      file.write(reinterpret_cast<const char*>(inventory->items), realInventorySlots * sizeof(Item));
   } else {
      Item item [realInventorySlots];
      file.write(reinterpret_cast<const char*>(item), realInventorySlots * sizeof(Item));
   }

   // Write console history
   size_t consoleHistorySize = (console ? console->history.size() : 0);
   file.write(reinterpret_cast<const char*>(&consoleHistorySize), sizeof(consoleHistorySize));
   if (console) {
      for (const std::string &string : console->history) {
         size_t size = string.size();
         file.write(reinterpret_cast<const char*>(&size), sizeof(size));
         file.write(reinterpret_cast<const char*>(string.data()), string.size());
      }
   }

   // Write the map
   int mapBlockCount = map.sizeX * map.sizeY;
   std::vector<blockid_t> blocks, walls;
   blocks.reserve(mapBlockCount);
   walls.reserve(mapBlockCount);

   // my dead ass simple compression algorithm that groups blocks together and writes the count and the ID. we then have
   // a super nice map.fill function to nicely fill these chunks after reading. since blockid_t is unsigned short and can
   // only hold a value up to 65k, we need to check against max. went from 9MB to ~250KB for a fresh world. a flat world
   // at creation is now under 1KB in size
   blockid_t lastBlock = (map.blocks.empty() ? 0 : map.blocks.front().id);
   blockid_t blockCount = 0;
   blockid_t blockMax = std::numeric_limits<blockid_t>::max();
   
   for (const Block &tile: map.blocks) {
      // basically set ID to 0 for ghost tiles (furniture). or get a corrupted world
      blockid_t id = (tile.tile == TileType::root) * tile.id;

      if (id == lastBlock && blockCount != blockMax) {
         blockCount += 1;
      }
      else {
         blocks.push_back(blockCount);
         blocks.push_back(lastBlock);
         lastBlock = id;
         blockCount = 1;
      }
   }
   blocks.push_back(blockCount);
   blocks.push_back(lastBlock);

   // do the same for walls...
   blockid_t lastWall = (map.walls.empty() ? 0 : map.walls.front().id);
   blockid_t wallCount = 0;
   blockid_t wallMax = std::numeric_limits<blockid_t>::max();

   for (const Wall &tile: map.walls) {
      if (tile.id == lastWall && wallCount != wallMax) {
         wallCount += 1;
      }
      else {
         walls.push_back(wallCount);
         walls.push_back(lastWall);
         lastWall = tile.id;
         wallCount = 1;
      }
   }
   walls.push_back(wallCount);
   walls.push_back(lastWall);

   // write size too since we have no idea how many chunks we'll get
   size_t blockSize = blocks.size();
   size_t wallSize = walls.size();
   file.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));
   file.write(reinterpret_cast<const char*>(&wallSize), sizeof(wallSize));
   file.write(reinterpret_cast<const char*>(blocks.data()), blocks.size() * sizeof(blockid_t));
   file.write(reinterpret_cast<const char*>(walls.data()), walls.size() * sizeof(blockid_t));

   // liquids are relatively scarce in the world, so 255 liquids per chunk is miniscule. we can optimize this just by saving
   // them as integers. flat maps will just save a single chunk of liquids, since they don't have any.
   std::vector<int> liquidHeights, liquidTypes;
   liquidHeights.reserve(mapBlockCount / 1000); // absurd to allocate all 1,5 million spots
   liquidTypes.reserve(mapBlockCount / 1000);

   // unlike we did for walls and blocks, we won't check for max here as 2 billion blocks is kind of a huge number. 2000x750
   // maps for comparison only have 1,5 million blocks. and they're pretty big.
   int lastLiquid = (map.liquidTypes.empty() ? 0 : map.liquidTypes.front());
   int liquidCount = 0;

   for (liquidid_t id: map.liquidTypes) {
      if (id == lastLiquid) {
         liquidCount += 1;
      }
      else {
         liquidTypes.push_back(liquidCount);
         liquidTypes.push_back(lastLiquid);
         lastLiquid = id;
         liquidCount = 1;
      }
   }
   liquidTypes.push_back(liquidCount);
   liquidTypes.push_back(lastLiquid);

   // do the same for liquid height...
   int lastLiquidHeight = (map.liquidHeights.empty() ? 0 : map.liquidHeights.front());
   int liquidHeightCount = 0;

   for (liquidlayer_t h: map.liquidHeights) {
      if (h == lastLiquidHeight) {
         liquidHeightCount += 1;
      }
      else {
         liquidHeights.push_back(liquidHeightCount);
         liquidHeights.push_back(lastLiquidHeight);
         lastLiquidHeight = h;
         liquidHeightCount = 1;
      }
   }
   liquidHeights.push_back(liquidHeightCount);
   liquidHeights.push_back(lastLiquidHeight);

   // and again write the size with the data.
   size_t liquidTypeSize = liquidTypes.size();
   size_t liquidHeightSize = liquidHeights.size();
   file.write(reinterpret_cast<const char*>(&liquidTypeSize), sizeof(liquidTypeSize));
   file.write(reinterpret_cast<const char*>(&liquidHeightSize), sizeof(liquidHeightSize));
   file.write(reinterpret_cast<const char*>(liquidTypes.data()), liquidTypes.size() * sizeof(int));
   file.write(reinterpret_cast<const char*>(liquidHeights.data()), liquidHeights.size() * sizeof(int));

   // Write the furniture
   size_t furnitureCount = map.furniture.size();
   file.write(reinterpret_cast<const char*>(&furnitureCount), sizeof(furnitureCount));

   for (const Furniture &obj: map.furniture) {
      if (obj.id == 0) continue; // we don't want any deleted furniture here.
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

   // everything's done
   auto end = std::chrono::steady_clock::now();
   file.close();
   size_t writeSize = std::filesystem::file_size(filename);
   printf("Successfully wrote %lluB (%lluKB) to 'data/worlds/%s.bin'. Took %lldms.\n", writeSize, writeSize / 1'000, name.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
}

void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map, Console &console, Inventory &inventory, std::vector<DroppedItem> &droppedItems) {
   auto begin = std::chrono::steady_clock::now();
   std::string filename = "data/worlds/" + name + ".bin";
   std::ifstream file (filename, std::ios::binary);

   if (!file.is_open()) {
      printf("loadWorldData: Failed to load world 'data/worlds/%s.bin'.\n", name.c_str());
      return;
   }

   // Read basic data
   int versionOfFile = 0;
   float timeofDay = 0;
   int moonPhase = 0;

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
   file.read(reinterpret_cast<char*>(&timeofDay), sizeof(timeofDay));
   file.read(reinterpret_cast<char*>(&moonPhase), sizeof(moonPhase));

   setTimeOfDay(timeofDay);
   setMoonPhase(moonPhase);
   map.init();

   // Read inventory
   file.read(reinterpret_cast<char*>(&inventory.items), realInventorySlots * sizeof(Item));

   // Read console
   size_t historySize = 0;
   file.read(reinterpret_cast<char*>(&historySize), sizeof(historySize));
   console.history.resize(historySize);

   for (size_t i = 0; i < historySize; ++i) {
      size_t lineSize = 0;
      file.read(reinterpret_cast<char*>(&lineSize), sizeof(lineSize));
      
      std::string line;
      line.resize(lineSize);
      file.read(reinterpret_cast<char*>(line.data()), lineSize);
      console.history[i] = line;
   }

   // read map. check saveWorldData for the compression algorithm in use
   size_t blockSize = 0;
   size_t wallSize = 0;
   file.read(reinterpret_cast<char*>(&blockSize), sizeof(size_t));
   file.read(reinterpret_cast<char*>(&wallSize), sizeof(size_t));

   std::vector<blockid_t> blocks, walls;
   blocks.resize(blockSize);
   walls.resize(wallSize);

   file.read(reinterpret_cast<char*>(blocks.data()), blocks.size() * sizeof(blockid_t));
   file.read(reinterpret_cast<char*>(walls.data()), walls.size() * sizeof(blockid_t));

   int blockCounter = 0;
   for (size_t i = 0; i < blockSize; i += 2) {
      blockid_t count = blocks[i];
      blockid_t id = blocks[i+1];
      map.fill(blockCounter, count, id);
      blockCounter += count;
   }

   int wallCounter = 0;
   for (size_t i = 0; i < wallSize; i += 2) {
      blockid_t count = walls[i];
      blockid_t id = walls[i+1];
      map.fillWalls(wallCounter, count, id);
      wallCounter += count;
   }

   // do the same with liquids...
   size_t liquidTypeSize = 0;
   size_t liquidHeightSize = 0;
   file.read(reinterpret_cast<char*>(&liquidTypeSize), sizeof(size_t));
   file.read(reinterpret_cast<char*>(&liquidHeightSize), sizeof(size_t));

   std::vector<int> liquidTypes, liquidHeights;
   liquidTypes.resize(liquidTypeSize);
   liquidHeights.resize(liquidHeightSize);

   file.read(reinterpret_cast<char*>(liquidTypes.data()), liquidTypes.size() * sizeof(int));
   file.read(reinterpret_cast<char*>(liquidHeights.data()), liquidHeights.size() * sizeof(int));

   int liquidTypeCounter = 0;
   for (size_t i = 0; i < liquidTypeSize; i += 2) {
      int count = liquidTypes[i];
      int id = liquidTypes[i+1];
      map.fillLiquids(liquidTypeCounter, count, id);
      liquidTypeCounter += count;
   }

   int liquidHeightCounter = 0;
   for (size_t i = 0; i < liquidHeightSize; i += 2) {
      int count = liquidHeights[i];
      int height = liquidHeights[i+1];
      map.fillLiquidHeights(liquidHeightCounter, count, height);
      liquidHeightCounter += count;
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

   // and read dropped items
   size_t droppedItemCount = 0;
   file.read(reinterpret_cast<char*>(&droppedItemCount), sizeof(droppedItemCount));
   droppedItems.resize(droppedItemCount);

   if (droppedItemCount > 0) {
      file.read(reinterpret_cast<char*>(droppedItems.data()), droppedItemCount * sizeof(DroppedItem));
   }
   player.init();

   // and that's done
   auto end = std::chrono::steady_clock::now();
   file.close();
   size_t writeSize = std::filesystem::file_size(filename);
   printf("Successfully read %lluB (%lluKB) from 'data/worlds/%s.bin'. Took %lldms.\n", writeSize, writeSize / 1'000, name.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
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
