#include "objs/map.hpp"
#include "SRU/assets.hpp"
#include "objs/inventory.hpp"
#include "objs/player.hpp"
#include "util/parallax.hpp"
#include "util/render.hpp"
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

BlockType getBlockType(blockid_t id) {
   return blockAttributes[id];
}

size_t getBlockCount() {
   return blockCount;
}

void reserveBlockContainers(size_t estimate) {
   blockNames.reserve(estimate);
   blockAttributes.reserve(estimate);
   blockIds.reserve(estimate);
}

size_t pushBlock(const std::string &name, BlockType attributes, Texture texture) {
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
   int oldI = oldY * sizeX + oldX;
   int newI = newY * sizeX + newX;
   std::swap(blocks[oldI], blocks[newI]);
   std::swap(liquidHeights[oldI], liquidHeights[newI]);
   std::swap(liquidTypes[oldI], liquidTypes[newI]);
}

void Map::addFurniture(Furniture &object) {
   for (int y = object.y; y < object.y + object.height; ++y) {
      for (int x = object.x; x < object.x + object.width; ++x) {
         FurniturePiece &piece = object.pieces[(y - object.y) * object.width + (x - object.x)];
         if (piece.nil) {
            continue;
         }
         int i = y * sizeX + x;
         blocks[i].tile = TileType::ghost;
         blocks[i].id = object.id;
         blocks[i].platformOverride = piece.walkable;
      }
   }
   furniture.push_back(object);
}

void Map::removeFurniture(Furniture &object) {
   for (int y = object.y; y < object.y + object.height; ++y) {
      for (int x = object.x; x < object.x + object.width; ++x) {
         if (!object.pieces[(y - object.y) * object.width + (x - object.x)].nil) {
            int i = y * sizeX + x;
            blocks[i].tile = TileType::root;
            blocks[i].id = 0;
            blocks[i].platformOverride = false;
         }
      }
   }
   object.deleted = true;
}

// getters

bool Map::isPositionValid(int x, int y) const {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::isPositionValid(Vector2 position) const {
   return isPositionValid(position.x, position.y);
}

bool Map::is(int x, int y, BlockType type) const {
   int i = y * sizeX + x;
   return isPositionValid(x, y) && blocks[i].tile == TileType::root && BlockTypeHas(blockAttributes[blocks[i].id], type);
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

bool Map::isLiquid(int x, int y) const {
   int i = y * sizeX + x;
   return isPositionValid(x, y) && liquidTypes[i] != LiquidType::none && liquidHeights[i] > minLiquidLayers;
}

bool Map::isAnyLiquid(int x, int y) const {
   return isPositionValid(x, y) && liquidTypes[y * sizeX + x] != LiquidType::none;
}

liquidlayer_t Map::getLiquidHeight(int x, int y) const {
   return liquidHeights[y * sizeX + x];
}

bool Map::isLiquidOfType(int x, int y, LiquidType type) const {
   return liquidTypes[y * sizeX + x] == type;
}

// render

void Map::renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color) {
   drawTexture(texture, {(((x + 0.5f - camera.target.x) * camera.zoom) + camera.offset.x) / 2.0f, (((y + 0.5f - camera.target.y) * camera.zoom) + camera.offset.y) / 2.0f}, size, 0, color);
}

void Map::render(const std::vector<DroppedItem> &droppedItems, const Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera, const Inventory &inventory) {
   // Render background walls
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         int i = y * sizeX + x;
         Block &wall = walls[i];
         BlockType wallType = blockAttributes[wall.id];
         BlockType blockType = blockAttributes[blocks[i].id];

         if (BlockTypeHas(wallType, BlockType::empty) || !BlockTypeHas(blockType, BlockType::translucent)) {
            continue;
         }

         int oldX = x;
         while (x <= cameraBounds.width && walls[y * sizeX + x].id == wall.id && BlockTypeHas(blockAttributes[blocks[y * sizeX + x].id], BlockType::translucent)) {
            x += 1;
         }

         drawTextureBlock(*wall.texture, {(float)oldX, (float)y, float(x - oldX), 1}, wallTint);
         x -= 1;
      }
   }

   // Render furniture
   for (const Furniture &obj: furniture) {
      obj.render(cameraBounds);
   }

   // Render blocks
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         int i = y * sizeX + x;
         const Block &block = blocks[i];
         BlockType blockType = blockAttributes[block.id];

         if (BlockTypeHas(blockType, BlockType::empty)) {
            continue;
         }
         // Render torches
         else if (BlockTypeHas(blockType, BlockType::torch)) {
            constexpr static float torchLightOffsetsY[] = {-1.0f, -1.0f * (5.0f / 8.0f), -0.75f, -0.75f, -1.0f * (5.0f / 8.0f)};

            float textureSize = block.texture->height / 2.0f;
            DrawTexturePro(*block.texture, {textureSize * block.value2, 0, textureSize, textureSize}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
            DrawTexturePro(*block.texture, {textureSize * block.value, textureSize, textureSize, textureSize}, {(float)x, (float)y + torchLightOffsetsY[block.value2], 1, 1}, {0, 0}, 0, WHITE);
            continue;
         }

         // Render regular blocks
         int oldX = x;
         while (x <= cameraBounds.width && blocks[y * sizeX + x].id == block.id) {
            x += 1;
         }

         int xx = x - oldX;
         drawTextureBlock(*block.texture, {(float)oldX, (float)y, (float)xx, 1});
         x -= 1;
      }
   }

   // Render the player
   if (player.hearts != 0) {
      player.render(accumulator, inventory.getCurrentToolsTexture());
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
         liquidFlags.r = (is(x, y - 1, BlockType::solid) && !is(x, y - 1, BlockType::platform) ? 255 : 0);
         liquidFlags.g = (!isAnyLiquid(x, y + 1) || !isLiquidOfType(x, y + 1, liquidTypes[y * sizeX + x]) ? 255 : 0);
         drawFluidBlock(getTexture("tiles"), {(float)x, (float)y + (1 - height), 1, height}, Fade(liquidFlags, height));
      }
   }
   EndShaderMode();

   // Render lights
   BeginTextureMode(lightmap);
   ClearBackground(BLACK);
   BeginBlendMode(BLEND_ADDITIVE);

   int lightBoundsMinX = std::max<int>(0, cameraBounds.x - 8);
   int lightBoundsMinY = std::max<int>(0, cameraBounds.y - 8);
   int lightBoundsMaxX = std::min<int>(sizeX - 1, cameraBounds.width + 8);
   int lightBoundsMaxY = std::min<int>(sizeY - 1, cameraBounds.height + 8);

   Color airLightColor   = getLightBasedOnTime();
   Color waterLightColor = Fade(airLightColor, 0.1f);

   // const function hack
   static float counter = 0.0f;
   counter += GetFrameTime();

   float sizeOffset     = std::sin(counter * 1.5f) * camera.zoom * 0.4f;
   float positionOffset = std::cos(counter * 0.8f) * camera.zoom * 0.0075f;

   Vector2 lightSize      = {3.5f * camera.zoom, 3.5f * camera.zoom};
   Vector2 lightLargeSize = {lightSize.x + lightSize.x, lightSize.y + lightSize.y};
   Vector2 lightHugeSize  = {lightLargeSize.x + lightSize.x, lightLargeSize.y + lightSize.y};
   Vector2 liquidSize     = {lightSize.x + sizeOffset, lightSize.y + sizeOffset};

   Texture2D &lightHugeTexture  = getTexture("lightsource_6x");
   Texture2D &lightLargeTexture = getTexture("lightsource_4x");
   Texture2D &lightTexture      = getTexture("lightsource_2x");

   for (int y = lightBoundsMinY; y <= lightBoundsMaxY; ++y) {
      for (int x = lightBoundsMinX; x <= lightBoundsMaxX; ++x) {
         BlockType type = blockAttributes[blocks[y * sizeX + x].id];
         if (!BlockTypeHas(type, BlockType::lightsource) && !BlockTypeHas(type, BlockType::translucent)) {
            continue;
         }

         // Direct light sources, ones who do not require the wall behind to be transparent
         if (isAnyLiquid(x, y) && isLiquidOfType(x, y, LiquidType::lava)) {
            if (isLiquid(x, y)) {
               renderLight(camera, lightTexture, x + positionOffset, y + positionOffset, liquidSize, {255, 125, 0, 255});
            }
            continue;
         } else if (BlockTypeHas(type, BlockType::torch)) {
            renderLight(camera, lightHugeTexture, x + positionOffset, y + positionOffset, lightHugeSize, {255, 200, 160, 255}); // Light orange
         } else if (BlockTypeHas(type, BlockType::lightsource)) {
            renderLight(camera, lightLargeTexture, x, y, lightLargeSize, {255, 255, 0, 255});
         }

         if (!BlockTypeHas(blockAttributes[walls[y * sizeX + x].id], BlockType::translucent)) {
            continue;
         }

         // Indirect light sources, these require to background to be empty
         if (isLiquid(x, y)) {
            renderLight(camera, lightTexture, x, y, lightSize, waterLightColor);
         } else {
            renderLight(camera, lightTexture, x, y, lightSize, airLightColor);
         }
      }
   }

   EndBlendMode();
   EndTextureMode();

   BeginBlendMode(BLEND_MULTIPLIED);
   DrawTexturePro(lightmap.texture, {0, 0, (float)lightmap.texture.width, -(float)lightmap.texture.height}, {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, {0, 0}, 0, WHITE);
   EndBlendMode();

   BeginMode2D(camera); // EndTextureMode disables it for some reason
}
