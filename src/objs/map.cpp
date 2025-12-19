#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/render.hpp"
#include <array>
#include <unordered_map>

// Constants

constexpr blockid_t blockCount = 20;
static inline std::unordered_map<std::string, blockid_t> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4},
   {"sand", 5}, {"sandstone", 6}, {"water", 7}, {"bricks", 8}, {"glass", 9},
   {"planks", 10}, {"stone_bricks", 11}, {"tiles", 12}, {"obsidian", 13}, {"lava", 14},
   {"platform", 15}, {"snow", 16}, {"ice", 17}, {"mud", 18}, {"jungle_grass", 19}
};

static inline std::array<const char*, blockCount> blockNames {
   "air", "grass", "dirt", "clay", "stone",
   "sand", "sandstone", "water", "bricks", "glass",
   "planks", "stone_bricks", "tiles", "obsidian", "lava",
   "platform", "snow", "ice", "mud", "jungle_grass"
};

static inline std::array<Block::Type, blockCount> blockTypes {{
   Block::air, Block::grass, Block::dirt, Block::solid, Block::solid,
   Block::sand, Block::solid, Block::water, Block::solid, Block::transparent,
   Block::solid, Block::solid, Block::solid, Block::solid, Block::lava,
   Block::platform, Block::snow, Block::ice, Block::dirt, Block::grass
}};

// Block functions

blockid_t Block::getId(const std::string &name) {
   return blockIds[name];
}

std::string Block::getName(blockid_t id) {
   return blockNames[id];
}

// Set block functions

void Map::init() {
   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
   walls  = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
}

void Map::setBlock(int x, int y, const std::string &name, bool isWall) {
   Block &block = (isWall ? walls : blocks)[y][x];
   
   block.id = blockIds[name];
   block.value = block.value2 = 0;
   block.type = blockTypes[block.id];

   if (block.id != 0) {
      block.texture = &getTexture(name);
   }
}

void Map::setBlock(int x, int y, blockid_t id, bool isWall) {
   setBlock(x, y, blockNames[id], isWall);
}

void Map::deleteBlock(int x, int y, bool wall) {
   Block &block = (wall ? walls : blocks)[y][x];
   block.texture = nullptr;
   block.type = Block::air;
   block.id = block.value = block.value2 = 0;
}

void Map::moveBlock(int oldX, int oldY, int newX, int newY) {
   std::swap(blocks[oldY][oldX], blocks[newY][newX]);
}

// Set furniture functions

void Map::addFurniture(Furniture &object) {
   for (int y = object.posY; y < object.sizeY + object.posY; ++y) {
      for (int x = object.posX; x < object.sizeX + object.posX; ++x) {
         if (!object.pieces[y - object.posY][x - object.posX].nil) {
            blocks[y][x].furniture = true;
         }
      }
   }
   furniture.push_back(object);
}

void Map::removeFurniture(Furniture &object) {
   for (int y = object.posY; y < object.sizeY + object.posY; ++y) {
      for (int x = object.posX; x < object.sizeX + object.posX; ++x) {
         if (!object.pieces[y - object.posY][x - object.posX].nil) {
            blocks[y][x].furniture = false;
         }
      }
   }
   object.deleted = true;
}

// Get block functions

bool Map::isPositionValid(int x, int y) {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::is(int x, int y, Block::Type type) {
   return isPositionValid(x, y) && blocks[y][x].type == type;
}

bool Map::isu(int x, int y, Block::Type type) {
   return blocks[y][x].type == type;
}

bool Map::empty(int x, int y) {
   return isPositionValid(x, y) and !blocks[y][x].furniture && blocks[y][x].type == Block::air;
}

bool Map::isTransparent(int x, int y) {
   Block::Type type = blocks[y][x].type;
   return type == Block::air || type == Block::water || type == Block::transparent || type == Block::platform;
}

std::vector<Block>& Map::operator[](size_t index) {
   return blocks[index];
}

// Render functions

void Map::render(const Rectangle &cameraBounds) {
   for (int y = cameraBounds.y; y < cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x < cameraBounds.width; ++x) {
         Block &wall = walls[y][x];
         if (wall.type == Block::air || !isTransparent(x, y)) {
            continue;
         }

         int oldX = x;
         while (x < cameraBounds.width && walls[y][x].id == wall.id && isTransparent(x, y)) {
            x += 1;
         }

         drawTextureBlock(*wall.texture, {(float)oldX, (float)y, float(x - oldX), 1}, wallTint);
         x -= 1;
      }
   }

   for (int y = cameraBounds.y; y < cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x < cameraBounds.width; ++x) {
         Block &block = blocks[y][x];
         if (block.type == Block::air) {
            continue;
         }

         int oldX = x;
         while (x < cameraBounds.width && blocks[y][x].id == block.id) {
            x += 1;
         }

         drawTextureBlock(*block.texture, {(float)oldX, (float)y, float(x - oldX), 1});
         x -= 1;
      }
   }

   for (Furniture &obj: furniture) {
      obj.render(cameraBounds.x, cameraBounds.y, cameraBounds.width, cameraBounds.height);
   }
}
