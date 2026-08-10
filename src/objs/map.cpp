#include "objs/map.hpp"
#include "SRU/assets.hpp"
#include <unordered_map>

// constants

static const std::unordered_map<std::string, BlockType> blockAttributeStrings {{
   {"empty", BlockType::empty}, {"grass", BlockType::grass}, {"dirt", BlockType::dirt}, {"sand", BlockType::sand},
   {"ice", BlockType::ice}, {"solid", BlockType::solid}, {"platform", BlockType::platform}, {"translucent", BlockType::translucent},
   {"lightsource", BlockType::lightsource}, {"torch", BlockType::torch}, {"flowable", BlockType::flowable}, {"sticky", BlockType::sticky},
   {"bouncy", BlockType::bouncy},
}};

// block info

static size_t blockCount = 0;
static std::vector<std::string> blockNames;
static std::vector<BlockType> blockAttributes;
static std::unordered_map<std::string, blockid_t> blockIds;

// Block getter functions

bool isBlockTypeValid(const std::string &name) {
   return blockAttributeStrings.find(name) != blockAttributeStrings.end();
}

BlockType getBlockTypeFromString(const std::string &name) {
   if (auto it = blockAttributeStrings.find(name); it != blockAttributeStrings.end()) {
      return it->second;
   }
   return BlockType::empty;
}

bool isBlockNameValid(const std::string &name) {
   return blockIds.find(name) != blockIds.end();
}

bool isBlockIdValid(blockid_t id) {
   return id >= 0 && id < blockCount;
}

blockid_t getBlockIdFromName(const std::string &name) {
   return blockIds.at(name);
}

std::string getBlockNameFromId(blockid_t id) {
   return blockNames[id];
}

size_t getBlockCount() {
   return blockCount;
}

void reserveBlockContainers(size_t estimate) {
   blockNames.reserve(estimate);
   blockAttributes.reserve(estimate);
   blockIds.reserve(estimate);
}

size_t pushBlock(const std::string &name, BlockType attributes) {
   blockNames.push_back(name);
   blockAttributes.push_back(attributes);
   blockIds[name] = blockCount;
   blockCount += 1;
   return blockCount - 1;
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
   Block block = Block{&getTexture(name), id, TileType::root};
   std::fill_n(&blocks[y * sizeX], sizeX, block);
}

void Map::setWallRow(int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   Block block = Block{&getTexture(name), id, TileType::root};
   std::fill_n(&walls[y * sizeX], sizeX, block);
}

void Map::setRow(int y, blockid_t *ids) {
   int start = y * sizeX;
   for (int i = 0; i < sizeX; ++i) {
      Block &block = blocks[start + i];
      block.id = ids[i];
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
      if (wall.id != 0) {
         wall.texture = &getTexture(blockNames[wall.id]);
      }
   }
}

void Map::setColumnFromPoint(int x, int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   Texture *texture = &getTexture(name);

   Block block {texture, id, TileType::root};
   Block wall {texture, id, TileType::root};

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

   if (!BlockTypeHas(blockAttributes[id], BlockType::flowable)) {
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
   return isPositionValid(x, y) && BlockTypeHas(blockAttributes[blocks[y * sizeX + x].id], type);
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
