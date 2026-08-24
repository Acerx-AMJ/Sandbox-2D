#include "objs/map.hpp"
#include "SRU/assets.hpp"
#include "SRU/render.hpp"
#include "SRU/util.hpp"
#include "objs/parallax.hpp"
#include "objs/player.hpp"
#include <unordered_map>

// constants

static const std::unordered_map<std::string, BlockType> blockAttributeStrings {{
   {"empty", BlockType::empty}, {"grass", BlockType::grass}, {"dirt", BlockType::dirt}, {"sand", BlockType::sand},
   {"ice", BlockType::ice}, {"solid", BlockType::solid}, {"platform", BlockType::platform}, {"translucent", BlockType::translucent},
   {"lightsource", BlockType::lightsource}, {"torch", BlockType::torch}, {"flowable", BlockType::flowable}, {"sticky", BlockType::sticky},
   {"bouncy", BlockType::bouncy},
}};

// block/liquid info

static size_t blockCount = 0;
static std::vector<std::string> blockNames;
static std::vector<BlockData> blockData;
static std::unordered_map<std::string, blockid_t> blockIds;

static size_t liquidCount = 1; // 1 - nil
static std::vector<std::string> liquidNames {""};
static std::vector<LiquidData> liquidData {{}};
static std::unordered_map<std::string, liquidid_t> liquidIds;

// Block getter functions

bool isBlockTypeValid(const std::string &name) {
   return blockAttributeStrings.find(name) != blockAttributeStrings.end();
}

bool isBlockNameValid(const std::string &name) {
   return blockIds.find(name) != blockIds.end();
}

bool isBlockIdValid(blockid_t id) {
   return id >= 0 && id < blockCount;
}

BlockType getBlockTypeFromString(const std::string &name) {
   if (auto it = blockAttributeStrings.find(name); it != blockAttributeStrings.end()) {
      return it->second;
   }
   return BlockType::empty;
}

BlockData &getBlockData(blockid_t id) {
   return blockData[id];
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
   blockNames.reserve(estimate + 1);
   blockData.reserve(estimate + 1);
   blockIds.reserve(estimate + 1);
}

void pushBlock(const std::string &name) {
   blockData.push_back({});
   blockNames.push_back(name);
   blockIds[name] = blockCount;
   blockCount += 1;
}

void setBlock(const std::string &name, BlockData data) {
   blockid_t id = blockIds.at(name);
   blockData[id] = data;
}

// liquid getter functions

bool isLiquidNameValid(const std::string &name) {
   return liquidIds.find(name) != liquidIds.end();
}

bool isLiquidIdValid(liquidid_t id) {
   return id >= 0 && id < liquidCount;
}

LiquidData &getLiquidData(liquidid_t id) {
   return liquidData[id];
}

liquidid_t getLiquidIdFromName(const std::string &name) {
   return liquidIds.at(name);
}

std::string getLiquidNameFromId(liquidid_t id) {
   return liquidNames[id];
}

size_t getLiquidCount() {
   return liquidCount;
}

void reserveLiquidContainers(size_t estimate) {
   liquidNames.reserve(estimate + 1);
   liquidData.reserve(estimate + 1);
   liquidIds.reserve(estimate + 1);
}

void pushLiquid(const std::string &name) {
   liquidData.push_back({});
   liquidNames.push_back(name);
   liquidIds[name] = liquidCount;
   liquidCount += 1;
}

void setLiquid(const std::string &name, const LiquidData &data) {
   liquidid_t id = liquidIds.at(name);
   liquidData[id] = data;
}

// constructors

void Map::init() {
   initThreadSafe();
   initContainers();
}

void Map::initThreadSafe() {
   waterTimeShaderLocation = GetShaderLocation(getShader("water"), "time");
}

void Map::initContainers() {
   const int area = sizeX * sizeY;
   blocks = std::vector<Block>(area, Block{});
   walls = std::vector<Wall>(area, Wall{});
   liquidHeights = std::vector<unsigned char>(area, 0);
   liquidTypes = std::vector<liquidid_t>(area, 0);
   lightmap = std::vector<unsigned char>(area, 0);
}

// setters

void Map::setRow(int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   Block block = {id, 0, TileType::root, blockData[id].attributes};
   std::fill_n(&blocks[y * sizeX], sizeX, block);
}

void Map::setWallRow(int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   Wall wall = {id, blockData[id].attributes};
   std::fill_n(&walls[y * sizeX], sizeX, wall);
}

void Map::setColumnFromPoint(int x, int y, const std::string &name) {
   blockid_t id = getBlockIdFromName(name);
   BlockType type = blockData[id].attributes;

   Block block {id, 0, TileType::root, type};
   Wall wall {id, type};

   int start = y * sizeX + x;
   int end = sizeX * sizeY;

   for (int i = start; i < end; i += sizeX) {
      blocks[i] = block;
      walls[i] = wall;
   }
}

void Map::fill(int i, int n, blockid_t id) {
   Block block = {id, 0, TileType::root, blockData[id].attributes};
   std::fill_n(&blocks[i], n, block);
}

void Map::fillWalls(int i, int n, blockid_t id) {
   Wall wall = {id, blockData[id].attributes};
   std::fill_n(&walls[i], n, wall);
}

void Map::fillLiquids(int i, int n, liquidid_t id) {
   std::fill_n(&liquidTypes[i], n, id);
}

void Map::fillLiquidHeights(int i, int n, liquidlayer_t height) {
   std::fill_n(&liquidHeights[i], n, height);
}

void Map::setBlock(int x, int y, const std::string &name) {
   setBlock(x, y, getBlockIdFromName(name));
}

void Map::setBlock(int x, int y, blockid_t id) {
   int i = y * sizeX + x;
   removeLight(i);
   Block &block = blocks[i];
   block.id = id;
   block.ghostId = 0;
   block.tile = TileType::root;
   block.type = blockData[id].attributes;
   block.value = 0;
   block.value2 = 0;
   block.platformOverride = false;

   if (!BlockTypeHas(block.type, BlockType::flowable)) {
      liquidHeights[i] = 0;
      liquidTypes[i] = 0;
   }
   addLight(i);
}

void Map::setWall(int x, int y, const std::string &name) {
   setWall(x, y, getBlockIdFromName(name));
}

void Map::setWall(int x, int y, blockid_t id) {
   int i = y * sizeX + x;
   removeLight(i);
   walls[i].id = id;
   walls[i].type = blockData[id].attributes;
   addLight(i);
}

void Map::setLiquid(int x, int y, liquidid_t id, liquidlayer_t height) {
   int i = y * sizeX + x;
   removeLight(i);
   liquidTypes[i] = id;
   liquidHeights[i] = height;
   addLight(i);
}

void Map::deleteBlock(int x, int y) {
   int i = y * sizeX + x;
   removeLight(i);
   blocks[i] = {};
   liquidHeights[i] = 0;
   liquidTypes[i] = 0;
   addLight(i);
}

void Map::deleteWall(int x, int y) {
   int i = y * sizeX + x;
   removeLight(i);
   walls[i] = {};
   addLight(i);
}

void Map::deleteBlockWithoutDeletingLiquids(int x, int y) {
   int i = y * sizeX + x;
   removeLight(i);
   blocks[i] = {};
   addLight(i);
}

void Map::swapBlocks(int oldX, int oldY, int newX, int newY) {
   int oldI = oldY * sizeX + oldX;
   int newI = newY * sizeX + newX;

   removeLight(oldI);
   removeLight(newI);

   std::swap(blocks[oldI], blocks[newI]);
   std::swap(liquidHeights[oldI], liquidHeights[newI]);
   std::swap(liquidTypes[oldI], liquidTypes[newI]);

   addLight(oldI);
   addLight(newI);
}

void Map::swapLiquids(int oldX, int oldY, int newX, int newY) {
   int oldI = oldY * sizeX + oldX;
   int newI = newY * sizeX + newX;

   removeLight(oldI);
   removeLight(newI);

   std::swap(liquidHeights[oldI], liquidHeights[newI]);
   std::swap(liquidTypes[oldI], liquidTypes[newI]);

   addLight(oldI);
   addLight(newI);
}

// lighting

unsigned char Map::getLightLevel(int i) {
   if (BlockTypeHas(walls[i].type, BlockType::translucent) && (BlockTypeHas(blocks[i].type, BlockType::lightsource) || BlockTypeHas(blocks[i].type, BlockType::translucent) || liquidTypes[i] != 0)) {
      return 4;
   }
   return 0;
}

unsigned char Map::getLightPassLevel(int i, int lightLevel) {
   int target = 1;
   if (liquidTypes[i] != 0) {
      target = 2;
   }
   else if (!BlockTypeHas(walls[i].type, BlockType::translucent) || !BlockTypeHas(blocks[i].type, BlockType::translucent)) {
      target = 1;
   }
   return fmax(0, lightLevel - target);
}

void Map::popLight() {
   while (!lightBfsQueue.empty()) {
      int i = lightBfsQueue.front();
      lightBfsQueue.pop();
      unsigned char a = lightmap[i];

      // left
      if (i % sizeX != 0 && lightmap[i - 1] + 2 <= a) {
         lightmap[i - 1] = getLightPassLevel(i - 1, a);
         lightBfsQueue.emplace(i - 1);
      }
      //right
      if (i % sizeX != sizeX - 1 && lightmap[i + 1] + 2 <= a) {
         lightmap[i + 1] = getLightPassLevel(i + 1, a);
         lightBfsQueue.emplace(i + 1);
      }
      // top
      if (i >= sizeX && lightmap[i - sizeX] + 2 <= a) {
         lightmap[i - sizeX] = getLightPassLevel(i - sizeX, a);
         lightBfsQueue.emplace(i - sizeX);
      }
      // bottom
      if (i < (sizeX * sizeY) - sizeX && lightmap[i + sizeX] + 2 <= a) {
         lightmap[i + sizeX] = getLightPassLevel(i + sizeX, a);
         lightBfsQueue.emplace(i + sizeX);
      }
   }
}

void Map::addLight(int i) {
   unsigned char lightLevel = getLightLevel(i);
   if (lightLevel == 0) return;

   lightmap[i] = lightLevel + 1;
   lightBfsQueue.emplace(i);
   popLight();
   lightmap[i] = lightLevel;
}

void Map::removeLight(int i) {
   unsigned char lightLevel = getLightLevel(i);
   lightRemovalBfsQueue.emplace(i, lightLevel);
   lightmap[i] = 0;

   while (!lightRemovalBfsQueue.empty()) {
      auto [i, v] = lightRemovalBfsQueue.front();
      lightRemovalBfsQueue.pop();

      // left
      if (i % sizeX != 0 && lightmap[i - 1] != 0 && lightmap[i - 1] < v) {
         unsigned char lightLevel = lightmap[i - 1];
         lightmap[i - 1] = 0;
         lightRemovalBfsQueue.emplace(i - 1, lightLevel);
      }
      else if (i % sizeX != 0 && lightmap[i - 1] >= v) {
         lightBfsQueue.emplace(i - 1);
      }
      //right
      if (i % sizeX != sizeX - 1 && lightmap[i + 1] != 0 && lightmap[i + 1] < v) {
         unsigned char lightLevel = lightmap[i + 1];
         lightmap[i + 1] = 0;
         lightRemovalBfsQueue.emplace(i + 1, lightLevel);
      }
      else if (i % sizeX != sizeX - 1 && lightmap[i + 1] >= v) {
         lightBfsQueue.emplace(i + 1);
      }
      // top
      if (i >= sizeX && lightmap[i - sizeX] != 0 && lightmap[i - sizeX] < v) {
         unsigned char lightLevel = lightmap[i - sizeX];
         lightmap[i - sizeX] = 0;
         lightRemovalBfsQueue.emplace(i - sizeX, lightLevel);
      }
      else if (i >= sizeX && lightmap[i - sizeX] >= v) {
         lightBfsQueue.emplace(i - sizeX);
      }
      // bottom
      if (i < (sizeX * sizeY) - sizeX && lightmap[i + sizeX] != 0 && lightmap[i + sizeX] < v) {
         unsigned char lightLevel = lightmap[i + sizeX];
         lightmap[i + sizeX] = 0;
         lightRemovalBfsQueue.emplace(i + sizeX, lightLevel);
      }
      else if (i < (sizeX * sizeY) - sizeX && lightmap[i + sizeX] >= v) {
         lightBfsQueue.emplace(i + sizeX);
      }
   }
   popLight();
}

void Map::calculateLighting() {
   int area = sizeX * sizeY;
   for (int i = 0; i < area; ++i) {
      addLight(i);
   }
   // for (int y = lightBoundsMinY; y <= lightBoundsMaxY; ++y) {
   //    for (int x = lightBoundsMinX; x <= lightBoundsMaxX; ++x) {
   //       BlockType type = blocks[y * sizeX + x].type;
   //       if (!BlockTypeHas(type, BlockType::lightsource) && !BlockTypeHas(type, BlockType::translucent)) {
   //          continue;
   //       }

   //       // Direct light sources, ones who do not require the wall behind to be transparent
   //       if (isAnyLiquid(x, y) && getLiquidData(x, y).glow) {
   //          if (isLiquid(x, y)) {
   //             renderLight(camera, lightTexture, x + positionOffset, y + positionOffset, liquidSize, {255, 125, 0, 255});
   //          }
   //          continue;
   //       } else if (BlockTypeHas(type, BlockType::torch)) {
   //          renderLight(camera, lightHugeTexture, x + positionOffset, y + positionOffset, lightHugeSize, {255, 200, 160, 255}); // Light orange
   //       } else if (BlockTypeHas(type, BlockType::lightsource)) {
   //          renderLight(camera, lightLargeTexture, x, y, lightLargeSize, {255, 255, 0, 255});
   //       }

   //       if (!BlockTypeHas(walls[y * sizeX + x].type, BlockType::translucent)) {
   //          continue;
   //       }

   //       // Indirect light sources, these require to background to be empty
   //       if (isLiquid(x, y) && getLiquidData(x, y).naturalLight) {
   //          renderLight(camera, lightTexture, x, y, lightSize, waterLightColor);
   //          continue;
   //       }
   //       renderLight(camera, lightTexture, x, y, lightSize, airLightColor);
   //    }
   // }
}

// furniture

void Map::updateFurniture(Player &player, Vector2 mousePos, float dt) {
   for (Furniture &object: furniture) {
      if (object.id != 0) {
         object.update(*this, player, mousePos, dt);
      }
   }
}

void Map::addFurniture(Furniture &object) {
   if (object.id == 0) return;
   size_t identifier = (furnitureEmptySlots.empty() ? furniture.size() : furnitureEmptySlots.back());
   object.mapIdentifier = identifier;

   for (int y = object.y; y < object.y + object.height; ++y) {
      for (int x = object.x; x < object.x + object.width; ++x) {
         FurniturePiece &piece = object.pieces[(y - object.y) * object.width + (x - object.x)];
         if (piece.nil) {
            continue;
         }
         int i = y * sizeX + x;
         blocks[i].tile = TileType::ghost;
         blocks[i].id = object.id;
         blocks[i].ghostId = identifier;
         blocks[i].type = blockData[0].attributes;
         blocks[i].platformOverride = piece.walkable;
      }
   }

   if (furnitureEmptySlots.empty()) {
      furniture.push_back(object);
   }
   else {
      furniture[identifier] = object;
      furnitureEmptySlots.pop_back();
   }
}

void Map::removeFurniture(Furniture &object) {
   for (int y = object.y; y < object.y + object.height; ++y) {
      for (int x = object.x; x < object.x + object.width; ++x) {
         if (!object.pieces[(y - object.y) * object.width + (x - object.x)].nil) {
            int i = y * sizeX + x;
            blocks[i].tile = TileType::root;
            blocks[i].id = 0;
            blocks[i].ghostId = 0;
            blocks[i].platformOverride = false;
         }
      }
   }
   furnitureEmptySlots.push_back(object.mapIdentifier);
   furniture[object.mapIdentifier].id = 0;
}

// getters

const Block &Map::getBlock(int x, int y) const {
   return blocks[y * sizeX + x];
}

const Wall &Map::getWall(int x, int y) const {
   return walls[y * sizeX + x];
}

bool Map::isPositionValid(int x, int y) const {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::isPositionValid(Vector2 position) const {
   return isPositionValid(position.x, position.y);
}

bool Map::is(int x, int y, BlockType type) const {
   int i = y * sizeX + x;
   return isPositionValid(x, y) && blocks[i].tile == TileType::root && BlockTypeHas(blocks[i].type, type);
}

bool Map::isWall(int x, int y, BlockType type) const {
   return isPositionValid(x, y) && BlockTypeHas(walls[y * sizeX + x].type, type);
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
   return is(x, y, BlockType::solid) && !is(x, y, BlockType::platform);
}

bool Map::blockNear(int x, int y) const {
   return x == 0 || x == sizeX - 1 || y == 0 || y == sizeY - 1 || !isNotSolid(x, y) || !isWall(x, y, BlockType::empty)
      || !isNotSolid(x + 1, y) || !isNotSolid(x - 1, y) || !isNotSolid(x, y + 1) || !isNotSolid(x, y - 1)
      || !isWall(x + 1, y, BlockType::empty) || !isWall(x - 1, y, BlockType::empty) || !isWall(x, y + 1, BlockType::empty) || !isWall(x, y - 1, BlockType::empty);
}

bool Map::isLiquid(int x, int y) const {
   int i = y * sizeX + x;
   return isPositionValid(x, y) && liquidTypes[i] != 0 && liquidHeights[i] > minLiquidLayers;
}

bool Map::isAnyLiquid(int x, int y) const {
   return isPositionValid(x, y) && liquidTypes[y * sizeX + x] != 0;
}

bool Map::isLiquidOfType(int x, int y, liquidid_t id) const {
   return liquidTypes[y * sizeX + x] == id;
}

liquidlayer_t Map::getLiquidHeight(int x, int y) const {
   return liquidHeights[y * sizeX + x];
}

liquidid_t Map::getLiquidId(int x, int y) const {
   return liquidTypes[y * sizeX + x];
}

LiquidData &Map::getLiquidData(int x, int y) const {
   return liquidData[liquidTypes[y * sizeX + x]];
}

// render

void Map::render(const std::vector<DroppedItem> &droppedItems, const Player &player, float accumulator, const Rectangle &cameraBounds) {
   // Render background walls
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         int i = y * sizeX + x;
         Wall &wall = walls[i];

         if (BlockTypeHas(wall.type, BlockType::empty) || !BlockTypeHas(blocks[i].type, BlockType::translucent)) {
            continue;
         }

         int oldX = x;
         while (x <= cameraBounds.width && walls[y * sizeX + x].id == wall.id && BlockTypeHas(blocks[y * sizeX + x].type, BlockType::translucent)) {
            x += 1;
         }

         Texture texture = blockData[wall.id].texture;
         Rectangle source = R4(0, 0, texture.width * (x - oldX), texture.height);
         drawTexture(texture, V2(oldX, y), V2(x - oldX, 1), TOP_LEFT, wallTint, source, 0.0f);
         x -= 1;
      }
   }

   // Render furniture
   for (const Furniture &obj: furniture) {
      if (obj.id != 0) {
         obj.render(cameraBounds);
      }
   }

   // Render blocks
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         int i = y * sizeX + x;
         Block &block = blocks[i];

         if (block.tile != TileType::root || BlockTypeHas(block.type, BlockType::empty)) {
            continue;
         }
         
         Texture texture = blockData[block.id].texture;
         if (BlockTypeHas(block.type, BlockType::torch)) {
            constexpr static float torchLightOffsetsY[] = {-1.0f, -1.0f * (5.0f / 8.0f), -0.75f, -0.75f, -1.0f * (5.0f / 8.0f)};

            float textureSize = texture.height / 2.0f;
            DrawTexturePro(texture, {textureSize * block.value2, 0, textureSize, textureSize}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
            DrawTexturePro(texture, {textureSize * block.value, textureSize, textureSize, textureSize}, {(float)x, (float)y + torchLightOffsetsY[block.value2], 1, 1}, {0, 0}, 0, WHITE);
            continue;
         }

         // Render regular blocks
         int oldX = x;
         while (x <= cameraBounds.width && blocks[y * sizeX + x].tile == TileType::root && blocks[y * sizeX + x].id == block.id) {
            x += 1;
         }

         Rectangle source = R4(0, 0, texture.width * (x - oldX), texture.height);
         drawTexture(texture, V2(oldX, y), V2(x - oldX, 1), TOP_LEFT, WHITE, source, 0.0f);
         x -= 1;
      }
   }

   // Render the player
   if (player.hearts != 0) {
      player.render(accumulator);
   }

   for (const DroppedItem &droppedItem : droppedItems) {
      droppedItem.render();
   }

   // Render fluids
   Shader &waterShader = getShader("water");
   float time = GetTime();
   SetShaderValue(waterShader, waterTimeShaderLocation, &time, SHADER_UNIFORM_FLOAT);
   BeginShaderMode(waterShader);

   // Render fluids
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         if (!isLiquid(x, y)) {
            continue;
         }

         float height = (float)getLiquidHeight(x, y) / (float)maxLiquidLayers;
         Color liquidFlags;
         liquidFlags.r = (isStable(x, y - 1) ? 255 : 0);
         liquidFlags.g = (!isAnyLiquid(x, y + 1) || !isLiquidOfType(x, y + 1, liquidTypes[y * sizeX + x]) ? 255 : 0);
         liquidFlags.b = height * 255.0f;
   
         Texture texture = getLiquidData(x, y).texture;
         Rectangle source = R4(0, texture.height - texture.height * height, texture.width, texture.height * height);
         drawTexture(texture, V2(x, y + (1 - height)), V2(1, height), TOP_LEFT, Fade(liquidFlags, height), source, 0.0f);
      }
   }
   EndShaderMode();

   // render lights
   unsigned char daylight = getLightBasedOnTime();
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         int lightLevel = lightmap[y * sizeX + x];
         int alpha = 255 - lightLevel * (255/4);
         int blended = fmin(255, alpha + daylight);
         DrawRectangleRec(R4(x, y, 1, 1), RGBA(0, 0, 0, blended));
      }
   }
}
