#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "objs/player.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/parallax.hpp"
#include "util/random.hpp"
#include <fstream>
#include <vector>

// File functions

std::string getRandomLineFromFile(const std::string &path) {
   std::fstream file (path.c_str());
   if (!file.is_open()) {
      return "";
   }

   std::vector<std::string> lines;
   std::string line;

   while (std::getline(file, line)) {
      lines.push_back(line);
   }
   return random(lines);
}

// World saving functions
// Save and load functions must follow the same data arrangement

void saveWorldData(const std::string &name, float playerX, float playerY, float zoom, const Map &map, const Inventory *inventory, const std::vector<DroppedItem> *droppedItems) {
   std::ofstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.txt'.", name);

   file << playerX << ' ';
   file << playerY << ' ';
   file << map.sizeX << ' ';
   file << map.sizeY << ' ';
   file << zoom << ' ';
   file << getLastTimeOfDay() << ' ';
   file << getLastMoonPhase() << ' ';

   if (inventory) {
      file << inventory->selectedX << ' ';
      file << inventory->selectedY << '\n';
   } else {
      file << 0 << ' ' << 0 << '\n';
   }

   for (int y = 0; y < inventoryHeight; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item item;
         if (inventory) {
            item = inventory->items[y][x];
         } else {
            item = Item{};
         }

         file << (int)item.type << ' ';
         file << (int)item.id << ' ';
         file << item.isFurniture << ' ';
         file << item.favorite << ' ';
         file << item.count << ' ';
      }
      file << '\n';
   }

   for (const std::vector<Block> &row: map.blocks) {
      for (const Block &tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }

   for (const std::vector<Block> &row: map.walls) {
      for (const Block &tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }

   file << map.furniture.size() << '\n';
   for (const Furniture &obj: map.furniture) {
      file << obj.posX << ' ';
      file << obj.posY << ' ';
      file << obj.sizeX << ' ';
      file << obj.sizeY << ' ';
      file << obj.type << ' ';
      file << (int)obj.value << ' ';
      file << (int)obj.value2 << ' ';
      file << (int)obj.texId << ' ';

      for (const std::vector<FurniturePiece> &row: obj.pieces) {
         for (const FurniturePiece &piece: row) {
            file << piece.nil << ' ';
            file << (int)piece.tx << ' ';
            file << (int)piece.ty << ' ';
         }
      }
      file << '\n';
   }

   if (droppedItems) {
      file << droppedItems->size() << '\n';
      for (const DroppedItem &item: *droppedItems) {
         file << (int)item.type << ' ';
         file << (int)item.id << ' ';
         file << item.isFurniture << ' ';
         file << item.count << ' ';
         file << item.tileX << ' ';
         file << item.tileY << ' ';
         file << item.lifetime << '\n';
      }
   } else {
      file << 0 << '\n';
   }
   file.close();
}

// World loading functions

void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map, Inventory &inventory, std::vector<DroppedItem> &droppedItems) {
   std::ifstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.txt'.", name);

   file >> player.position.x;
   file >> player.position.y;
   file >> map.sizeX;
   file >> map.sizeY;
   file >> zoom;
   file >> getTimeOfDay();
   file >> getMoonPhase();
   file >> inventory.selectedX;
   file >> inventory.selectedY;
   map.init();

   for (int y = 0; y < inventoryHeight; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item &item = inventory.items[y][x];

         int type = 0;
         file >> type;
         item.type = (Item::Type)type;

         int newId = 0;
         file >> newId;
         item.id = newId;

         file >> item.isFurniture;
         file >> item.favorite;
         file >> item.count;
      }
   }

   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (blockid_t)id);
      }
   }

   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (blockid_t)id, true);
      }
   }

   // The beast behind loading furniture
   int furnitureCount = 0;
   file >> furnitureCount;

   for (int i = 0; i < furnitureCount; ++i) {
      int posX = 0, posY = 0, sizeX = 0, sizeY = 0, type = 0, value = 0, value2 = 0, texId = 0;
      file >> posX;
      file >> posY;
      file >> sizeX;
      file >> sizeY;
      file >> type;
      file >> value;
      file >> value2;
      file >> texId;

      Furniture furniture ((Furniture::Type)type, texId, value, value2, posX, posY, sizeX, sizeY);
      for (int y = 0; y < sizeY; ++y) {
         for (int x = 0; x < sizeX; ++x) {
            FurniturePiece &piece = furniture.pieces[y][x];

            int nil = 0, tx = 0, ty = 0;
            file >> nil;
            file >> tx;
            file >> ty;
            piece = {(unsigned char)tx, (unsigned char)ty, (bool)nil};
         }
      }
      map.addFurniture(furniture);
   }

   int droppedItemCount = 0;
   file >> droppedItemCount;

   for (int i = 0; i < droppedItemCount; ++i) {
      int type = 0, id = 0, isFurniture = 0, count = 0, tileX = 0, tileY = 0;
      float lifetime = 0.0f;
      file >> type;
      file >> id;
      file >> isFurniture;
      file >> count;
      file >> tileX;
      file >> tileY;
      file >> lifetime;

      DroppedItem item {(Item::Type)type, (unsigned char)id, (bool)isFurniture, count, tileX, tileY, lifetime};
      droppedItems.push_back(item);
   }
   player.init();
}
