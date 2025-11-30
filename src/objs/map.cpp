#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <array>
#include <unordered_map>

// Constants

constexpr Block::id_t idCount = 18;
constexpr Color backgroundTint {120, 120, 120, 255};

static std::unordered_map<std::string, Block::id_t> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4},
   {"sand", 5}, {"sandstone", 6}, {"water", 7}, {"bricks", 8}, {"glass", 9},
   {"planks", 10}, {"stone_bricks", 11}, {"tiles", 12}, {"obsidian", 13}, {"lava", 14},
   {"platform", 15}, {"snow", 16}, {"ice", 17}
};

constexpr static std::array<const char*, idCount> blockNames {
   "air", "grass", "dirt", "clay", "stone",
   "sand", "sandstone", "water", "bricks", "glass",
   "planks", "stone_bricks", "tiles", "obsidian", "lava",
   "platform", "snow", "ice"
};

constexpr static std::array<Block::Type, idCount> blockTypes {{
   Block::air, Block::grass, Block::dirt, Block::solid, Block::solid,
   Block::sand, Block::solid, Block::water, Block::solid, Block::transparent,
   Block::solid, Block::solid, Block::solid, Block::solid, Block::lava,
   Block::platform, Block::snow, Block::ice
}};

static std::array<Color, idCount> wallColors, blockColors;

// Block functions

Color& Block::getColor() {
   return blockColors[id];
}

Color& Block::getWallColor() {
   return wallColors[id];
}

void Block::initializeColors() {
   for (const auto& [name, id]: blockIds) {
      auto& texture = getTexture(name);
      auto image = LoadImageFromTexture(texture);
      Color a = GetImageColor(image, 0, 0);

      // Ignore any transparent blocks (like glass)
      if (a.a == 0) {
         blockColors[id] = wallColors[id] = a;
         UnloadImage(image);
         continue;
      }

      // Do some cheats to have our color tinted without having to implement
      // any math ourselves
      auto target = LoadRenderTexture(1, 1);
      BeginTextureMode(target);
      DrawTexture(texture, 0, 0, backgroundTint);
      EndTextureMode();

      auto image2 = LoadImageFromTexture(target.texture);
      Color b = GetImageColor(image2, 0, 0);

      blockColors[id] = a;
      wallColors[id] = b;

      UnloadImage(image);
      UnloadImage(image2);
      UnloadRenderTexture(target);
   }
}

Block::id_t Block::getId(const std::string& name) {
   return blockIds[name];
}

Color& Block::getColorFromId(Block::id_t id) {
   return blockColors[id];
}

// Set block functions

void Map::init() {
   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
   walls = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
}

void Map::setBlock(int x, int y, const std::string& name, bool wall) {
   auto& block = (wall ? walls : blocks)[y][x];
   
   block.id = blockIds[name];
   block.value = block.value2 = 0;
   block.type = blockTypes[block.id];

   if (block.id != 0) {
      block.tex = &getTexture(name);
   }
}

void Map::setBlock(int x, int y, Block::id_t id, bool wall) {
   auto& block = (wall ? walls : blocks)[y][x];
   block.id = id;
   block.type = blockTypes[block.id];

   if (block.id != 0) {
      block.tex = &getTexture(blockNames[id]);
   }
}

void Map::deleteBlock(int x, int y, bool wall) {
   auto& block = (wall ? walls : blocks)[y][x];
   block.tex = nullptr;
   block.type = Block::air;
   block.id = block.value = block.value2 = 0;
}

void Map::moveBlock(int ox, int oy, int nx, int ny) {
   std::swap(blocks[oy][ox], blocks[ny][nx]);
}

// Set furniture functions

void Map::addFurniture(Furniture& obj) {
   for (int y = obj.posY; y < obj.sizeY + obj.posY; ++y) {
      for (int x = obj.posX; x < obj.sizeX + obj.posX; ++x) {
         if (not obj.pieces[y - obj.posY][x - obj.posX].nil) {
            blocks[y][x].furniture = true;
         }
      }
   }
   furniture.push_back(obj);
}

void Map::removeFurniture(Furniture& obj) {
   for (int y = obj.posY; y < obj.sizeY + obj.posY; ++y) {
      for (int x = obj.posX; x < obj.sizeX + obj.posX; ++x) {
         if (not obj.pieces[y - obj.posY][x - obj.posX].nil) {
            blocks[y][x].furniture = false;
         }
      }
   }
   obj.deleted = true;
}

// Get block functions

bool Map::isPositionValid(int x, int y) {
   return x >= 0 and x < sizeX and y >= 0 and y < sizeY;
}

bool Map::is(int x, int y, Block::Type type) {
   return isPositionValid(x, y) and blocks[y][x].type == type;
}

bool Map::isu(int x, int y, Block::Type type) {
   return blocks[y][x].type == type;
}

bool Map::empty(int x, int y) {
   return not blocks[y][x].furniture and blocks[y][x].type == Block::air;
}

bool Map::isTransparent(int x, int y) {
   if (not isPositionValid(x, y)) {
      return false;
   }
   auto t = blocks[y][x].type;
   return t == Block::air or t == Block::water or t == Block::transparent or t == Block::platform;
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

   for (auto& obj: furniture) {
      obj.render(camera.zoom <= 12.5f, minX, minY, maxX, maxY);
   }
}
