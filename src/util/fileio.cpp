#include "objs/map.hpp"
#include "objs/player.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
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
// Save and load functions must follow the same data arrangement (duh)

void saveWorldData(const std::string &name, float playerX, float playerY, float zoom, const Map &map) {
   std::ofstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.txt'.", name);

   file << playerX << ' ';
   file << playerY << ' ';
   file << map.sizeX << ' ';
   file << map.sizeY << ' ';
   file << zoom << '\n';

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
   file.close();
}

// World loading functions

void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map) {
   std::ifstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.txt'.", name);

   file >> player.position.x;
   file >> player.position.y;
   file >> map.sizeX;
   file >> map.sizeY;
   file >> zoom;
   map.init();

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
   int posX = 0;
   while (file >> posX) {
      int posY = 0, sizeX = 0, sizeY = 0, type = 0, value = 0, value2 = 0, texId = 0;
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
   player.init();
}
