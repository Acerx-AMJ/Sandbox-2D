#include "objs/map.hpp"
#include "mngr/resource.hpp"
#include "util/strarray.hpp"

// block info

constexpr size_t blockCount = 30;
constexpr static inline const char* blockNames[blockCount] {};
constexpr static inline BlockType blockAttributes[blockCount] {};
const static inline StrArray<std::string> blockIds {};

// Block getter functions

bool isBlockNameValid(const std::string &name) {
   return blockIds.map.find(name) != blockIds.map.end();
}

bool isBlockIdValid(blockid_t id) {
   return id >= 0 && id < blockCount;
}

blockid_t getBlockIdFromName(const std::string &name) {
   return blockIds.map.at(name);
}

std::string getBlockNameFromId(blockid_t id) {
   return blockNames[id];
}

size_t getBlockCount() {
   return blockCount;
}

// constructors

void Map::init() {
   initThreadSafe();
   initContainers();
}

void Map::initThreadSafe() {
   waterTimeShaderLocation = GetShaderLocation(getShader("water"), "time");
   lightmap = LoadRenderTexture(GetScreenWidth() / 2, GetScreenHeight() / 2);
}

void Map::initContainers() {
   const int area = sizeX * sizeY;
   blocks = std::vector<Block>(area, Block{});
   walls = std::vector<Block>(area, Block{});
   liquidHeights = std::vector<unsigned char>(area, 0);
   liquidTypes = std::vector<LiquidType>(area, LiquidType::none);
}

Map::~Map() {
   UnloadRenderTexture(lightmap);
}

// setters

void Map::setRow(int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   BlockType type = blockAttributes[id];
   Block block = Block{&getTexture(name), id, TileType::root, type};
   std::fill_n(&blocks[y * sizeX], sizeX, block);
}

void Map::setWallRow(int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   BlockType type = blockAttributes[id];
   Block block = Block{&getTexture(name), id, TileType::root, type};
   std::fill_n(&walls[y * sizeX], sizeX, block);
}

void Map::setRow(int y, blockid_t *ids) {
   int start = y * sizeX;
   for (int i = 0; i < sizeX; ++i) {
      Block &block = blocks[start + i];
      block.id = ids[i];
      block.type = blockAttributes[block.id];
      if (block.id != 0) {
         block.texture = &getTexture(blockNames[block.id]);
      }
   }
}

void Map::setWallRow(int y, blockid_t *ids) {
   int start = y * sizeX;
   for (int i = 0; i < sizeX; ++i) {
      Block &wall = walls[start + i];
      wall.id = ids[i];
      wall.type = blockAttributes[wall.id];
      if (wall.id != 0) {
         wall.texture = &getTexture(blockNames[wall.id]);
      }
   }
}

void Map::setColumnFromPoint(int x, int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   BlockType type = blockAttributes[id];
   Texture *texture = &getTexture(name);

   Block block {texture, id, TileType::root, type};
   Block wall {texture, id, TileType::root, type};

   int start = y * sizeX + x;
   int end = sizeX * sizeY;

   for (int i = start; i < end; i += sizeX) {
      blocks[i] = block;
      walls[i] = wall;
   }
}

void Map::setBlock(int x, int y, const std::string &name) {
   setBlock(x, y, getBlockIdFromName(name));
}

void Map::setBlock(int x, int y, blockid_t id) {
   int i = y * sizeX + x;
   Block &block = blocks[i];
   block.id = id;
   block.value = 0;
   block.value2 = 0;
   block.type = blockAttributes[block.id];

   if (!BlockTypeHas(block.type, BlockType::flowable)) {
      liquidHeights[i] = 0;
      liquidTypes[i] = LiquidType::none;
   }

   if (id != 0) {
      block.texture = &getTexture(blockNames[id]);
   }
}

void Map::setWall(int x, int y, const std::string &name) {
   setWall(x, y, getBlockIdFromName(name));
}

void Map::setWall(int x, int y, blockid_t id) {
   int i = y * sizeX + x;
   walls[i].id = id;
   if (id != 0) {
      walls[i].texture = &getTexture(blockNames[id]);
   }
}

void Map::deleteBlock(int x, int y) {
   int i = y * sizeX + x;
   blocks[i] = {};
   liquidHeights[i] = 0;
   liquidTypes[i] = LiquidType::none;
}

void Map::deleteWall(int x, int y) {
   walls[y * sizeX + x] = {};
}

void Map::deleteBlockWithoutDeletingLiquids(int x, int y) {
   blocks[y * sizeX + x] = {};
}

void Map::swapBlocks(int oldX, int oldY, int newX, int newY) {
   int oldI = oldY * sizeY + oldX;
   int newI = newY * sizeY + newX;
   std::swap(blocks[oldI], blocks[newI]);
   std::swap(liquidHeights[oldI], liquidHeights[newI]);
   std::swap(liquidTypes[oldI], liquidTypes[newI]);
}

void Map::addFurniture(Furniture &object) {

}

void Map::removeFurniture(Furniture &object) {

}

// getters

bool Map::isPositionValid(int x, int y) const {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::isPositionValid(Vector2 position) const {
   return isPositionValid(position.x, position.y);
}

bool Map::is(int x, int y, BlockType type) const {
   return isPositionValid(x, y) && BlockTypeHas(blocks[y * sizeX + x].type, type);
}

bool Map::isSoil(int x, int y) const {
   return is(x, y, BlockType::grass) || is(x, y, BlockType::dirt) || is(x, y, BlockType::sand);
}

bool Map::isEmpty(int x, int y) const {
   return is(x, y, BlockType::empty) && !isLiquid(x, y);
}

bool Map::isNotSolid(int x, int y) const {
   return is(x, y, BlockType::empty);
}

bool Map::isStable(int x, int y) const {
   return is(x, y, BlockType::solid) || !is(x, y, BlockType::platform);
}

bool Map::isLiquid(int x, int y) const {
   int i = y * sizeX + x;
   return isPositionValid(x, y) && liquidTypes[i] != LiquidType::none && liquidHeights[i] > minLiquidLayers;
}

bool Map::isAnyLiquid(int x, int y) const {
   return isPositionValid(x, y) && liquidTypes[y * sizeX + x] != LiquidType::none;
}

liquidlayer_t Map::getLiquidHeight(int x, int y) const {
   return liquidHeights[y * sizeY + x];
}

bool Map::isLiquidOfType(int x, int y, LiquidType type) const {
   return liquidTypes[y * sizeY + x] == type;
}

// render

void render(const std::vector<struct DroppedItem> &droppedItems, const struct Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera, const Inventory &inventory) {

}
