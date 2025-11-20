#include <array>
#include <unordered_map>
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/format.hpp" // IWYU pragma: export
#include "util/math.hpp"
#include "util/position.hpp"
#include "util/render.hpp"

// Constants

constexpr int idCount = 13;
constexpr Color backgroundTint {90, 90, 90, 255};

static std::unordered_map<std::string, int> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4},
   {"sand", 5}, {"sandstone", 6}, {"water", 7}, {"bricks", 8}, {"glass", 9},
   {"planks", 10}, {"stone_bricks", 11}, {"tiles", 12}
};

static std::array<Block::Type, idCount> blockTypes {{
   Block::air, Block::grass, Block::dirt, Block::solid, Block::solid,
   Block::sand, Block::solid, Block::water, Block::solid, Block::transparent,
   Block::solid, Block::solid, Block::solid
}};

static std::array<Color, idCount> wallColors, blockColors {{
   {0, 0, 0, 0}, {28, 152, 29, 255}, {117, 56, 19, 255}, {158, 91, 35, 255}, {102, 102, 102, 255},
   {255, 189, 40, 255}, {247, 134, 13, 255}, {8, 69, 165, 255}, {244, 52, 8, 255}, {0, 0, 0, 0},
   {158, 91, 35, 255}, {102, 102, 102, 255}, {102, 102, 102, 255}
}};

// Block functions

Color& Block::getColor() {
   return blockColors[id];
}

Color& Block::getWallColor() {
   return wallColors[id];
}

void Block::initializeWallColors() {
   for (int i = 0; i < idCount; ++i) {
      Color& a = blockColors[i];
      Color b = backgroundTint;
      Color c {(unsigned char)clamp(a.r - b.r, 0, 255), (unsigned char)clamp(a.g - b.g, 0, 255), (unsigned char)clamp(a.b - b.b, 0, 255), a.a};
      wallColors[i] = c;
   }
}

// Set block functions

void Map::setSize(int x, int y) {
   sizeX = x;
   sizeY = y;
   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
   walls = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
}

void Map::setBlock(int x, int y, const std::string& name, bool wall) {
   assert(blockIds.find(name) != blockIds.end(), "Block with the name '{}' does not exist.", name);
   auto& block = (wall ? walls : blocks)[y][x];
   
   block.tex = &ResourceManager::get().getTexture(name);
   block.id = blockIds[name];
   block.type = blockTypes[block.id];
}

void Map::deleteBlock(int x, int y, bool wall) {
   auto& block = (wall ? walls : blocks)[y][x];
   block.tex = nullptr;
   block.type = Block::air;
   block.id = 0;
}

void Map::moveBlock(int ox, int oy, int nx, int ny) {
   std::swap(blocks[oy][ox], blocks[ny][nx]);
}

// Get block functions

bool Map::isPositionValid(int x, int y) {
   return x >= 0 and x < sizeX and y >= 0 and y < sizeY;
}

bool Map::is(int x, int y, Block::Type type) {
   return isPositionValid(x, y) and blocks[y][x].type == type;
}

bool Map::isTransparent(int x, int y) {
   return is(x, y, Block::air) or is(x, y, Block::transparent) or is(x, y, Block::water);
}

std::vector<Block>& Map::operator[](size_t index) {
   return blocks[index];
}

// Render functions

void Map::render(Camera2D& camera) {
   auto bounds = getCameraBounds(camera);

   auto minX = std::max(0, int(bounds.x));
   auto minY = std::max(0, int(bounds.y));
   auto maxX = std::min(sizeX, int((bounds.x + bounds.width)) + 1);
   auto maxY = std::min(sizeY, int((bounds.y + bounds.height)) + 1);

   for (int y = minY; y < maxY; ++y) {
      for (int x = minX; x < maxX; ++x) {
         auto& wall = walls[y][x];
         if (wall.type == Block::air or not isTransparent(x, y)) {
            continue;
         }

         int ox = x;
         while (x < maxX and walls[y][x].id == wall.id and isTransparent(x, y)) { ++x; }

         if (camera.zoom <= 12.5f) {
            DrawRectangle(ox, y, x - ox, 1, wall.getWallColor());
         } else {
            drawTextureBlock(*wall.tex, {(float)ox, (float)y, float(x - ox), 1.f}, backgroundTint);
         }
         --x;
      }
   }

   for (int y = minY; y < maxY; ++y) {
      for (int x = minX; x < maxX; ++x) {
         auto& block = blocks[y][x];
         if (block.type == Block::air) {
            continue;
         }

         int ox = x;
         while (x < maxX and blocks[y][x].id == block.id) { ++x; }

         if (camera.zoom <= 12.5f) {
            DrawRectangle(ox, y, x - ox, 1, block.getColor());
         } else {
            drawTextureBlock(*block.tex, {(float)ox, (float)y, float(x - ox), 1.f});
         }
         --x;
      }
   }
}
