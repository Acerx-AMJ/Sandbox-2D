#include "objs/map.hpp"
#include "objs/player.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/random.hpp"
#include <fstream>
#include <vector>

// File functions

std::string getRandomLineFromFile(const std::filesystem::path& path) {
   std::fstream file (path.string().c_str());
   if (not file.is_open()) {
      return "";
   }

   std::vector<std::string> lines;
   std::string line;

   while (std::getline(file, line)) {
      lines.push_back(line);
   }
   return lines[random(0, lines.size() - 1)];
}

// World saving functions
// Save and load functions must follow the same data arrangement (duh)

void saveWorldData(const std::string& name, float playerX, float playerY, float zoom, const Map& map) {
   std::ofstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.txt'.", name);
   file << playerX << ' ' << playerY << ' ' << map.sizeX << ' ' << map.sizeY << ' ' << zoom << '\n';

   for (const auto& row: map.blocks) {
      for (const auto& tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }

   for (const auto& row: map.walls) {
      for (const auto& tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }

   for (const auto& obj: map.furniture) {
      file << obj.posX << ' ' << obj.posY << ' ' << obj.sizeX << ' ' << obj.sizeY << ' ' << obj.type << ' ' << (int)obj.value << ' ' << (int)obj.value2 << ' ' << (int)obj.texId << ' ';
      for (const auto& row: obj.pieces) {
         for (const auto& piece: row) {
            file << piece.nil << ' ' << (int)piece.colorId << ' ' << (int)piece.tx << ' ' << (int)piece.ty << ' ';
         }
      }
      file << '\n';
   }
   file.close();
}

// World loading functions

void loadWorldData(const std::string& name, Player& player, float& zoom, Map& map) {
   std::ifstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.txt'.", name);

   file >> player.pos.x >> player.pos.y;
   file >> map.sizeX >> map.sizeY;
   file >> zoom;
   map.init();

   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (Block::id_t)id);
      }
   }

   // Do the same for background walls
   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (Block::id_t)id, true);
      }
   }

   // The beast behind loading furniture
   int posX = 0;
   while (file >> posX) {
      int posY = 0, sizeX = 0, sizeY = 0, type = 0, value = 0, value2 = 0, texId = 0;
      file >> posY >> sizeX >> sizeY >> type >> value >> value2 >> texId;

      Furniture furniture ((Furniture::Type)type, texId, value, value2, posX, posY, sizeX, sizeY);
      for (int y = 0; y < sizeY; ++y) {
         for (int x = 0; x < sizeX; ++x) {
            auto& piece = furniture.pieces[y][x];
            int nil = 0, colorId = 0, tx = 0, ty = 0;
            file >> nil >> colorId >> tx >> ty;
            piece = {(unsigned char)tx, (unsigned char)ty, (unsigned char)colorId, (bool)nil};
         }
      }
      map.addFurniture(furniture);
   }
   player.init();
}
