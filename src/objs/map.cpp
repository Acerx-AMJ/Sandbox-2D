#include "game/state.hpp"
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "objs/inventory.hpp"
#include "objs/item.hpp"
#include "objs/player.hpp"
#include "util/format.hpp"
#include "util/parallax.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include "util/strarray.hpp"
#include <array>
#include <cmath>
#include <raymath.h>

// Constants

constexpr unsigned short blockCount = 30;

struct BlockInfo {
   const char *name;
   BlockType attributes;
   float breakTime;
   int breakLevel;
   Item drop;
};

// NOTE: due to logic in gameState.cpp, any grass blocks must be defined RIGHT
// BEFORE the dirt block, for an example, you can see blocks 1 and 2
constexpr static inline std::array<BlockInfo, blockCount> blockInfo {

   //        texture name          attributes                                                                                                 break speed   break level   drop
   //*************************************************************************************************************************************************************************
   BlockInfo{"air",                BlockType::empty | BlockType::transparent | BlockType::flowable,                                           0.0f,         0,            Item{}},
   BlockInfo{"grass",              BlockType::solid | BlockType::grass,                                                                       0.75f,        0,            Item{}},
   BlockInfo{"dirt",               BlockType::solid | BlockType::dirt,                                                                        0.75f,        0,            Item{}},
   BlockInfo{"clay",               BlockType::solid,                                                                                          1.0f,         0,            Item{}},
   BlockInfo{"stone",              BlockType::solid,                                                                                          2.0f,         1,            Item{}},
   BlockInfo{"sand",               BlockType::solid | BlockType::sand,                                                                        0.5f,         0,            Item{}},
   BlockInfo{"sandstone",          BlockType::solid,                                                                                          1.5f,         1,            Item{}},
   BlockInfo{"bricks",             BlockType::solid,                                                                                          2.5f,         2,            Item{}},
   BlockInfo{"glass",              BlockType::solid | BlockType::transparent,                                                                 0.25f,        0,            Item{}},
   BlockInfo{"planks",             BlockType::solid,                                                                                          1.5f,         0,            Item{}},
   BlockInfo{"stone_bricks",       BlockType::solid,                                                                                          3.5f,         2,            Item{}},
   BlockInfo{"tiles",              BlockType::solid,                                                                                          3.5f,         2,            Item{}},
   BlockInfo{"obsidian",           BlockType::solid,                                                                                          2.5f,         3,            Item{}},
   BlockInfo{"platform",           BlockType::solid | BlockType::platform | BlockType::transparent | BlockType::flowable,                     1.0f,         1,            Item{}},
   BlockInfo{"snow",               BlockType::solid | BlockType::sand,                                                                        0.5f,         0,            Item{}},
   BlockInfo{"ice",                BlockType::solid | BlockType::ice,                                                                         1.5f,         1,            Item{}},
   BlockInfo{"jungle_grass",       BlockType::solid | BlockType::grass,                                                                       0.75f,        0,            Item{}},
   BlockInfo{"mud",                BlockType::solid | BlockType::dirt,                                                                        0.75f,        0,            Item{}},
   BlockInfo{"lamp",               BlockType::solid | BlockType::lightsource,                                                                 1.0f,         0,            Item{}},
   BlockInfo{"torch",              BlockType::transparent | BlockType::lightsource | BlockType::torch | BlockType::flowable,                  0.25f,        0,            Item{}},
   BlockInfo{"honey_block",        BlockType::solid | BlockType::sticky,                                                                      1.5f,         0,            Item{}},
   BlockInfo{"crispy_honey_block", BlockType::solid,                                                                                          2.0f,         0,            Item{}},
   BlockInfo{"slime_block",        BlockType::solid | BlockType::bouncy | BlockType::transparent,                                             1.0f,         0,            Item{}},
   BlockInfo{"bubble_block",       BlockType::transparent,                                                                                    0.25f,        0,            Item{}},
   BlockInfo{"slime_platform",     BlockType::platform | BlockType::transparent | BlockType::solid | BlockType::flowable | BlockType::bouncy, 0.75f,        0,            Item{}},
   BlockInfo{"cactus_block",       BlockType::solid,                                                                                          0.75f,        0,            Item{}},
   BlockInfo{"coal_ore",           BlockType::solid,                                                                                          2.5f,         1,            Item{ItemType::item, 1, 1}},
   BlockInfo{"iron_ore",           BlockType::solid,                                                                                          2.5f,         2,            Item{ItemType::item, 2, 1}},
   BlockInfo{"gold_ore",           BlockType::solid,                                                                                          3.0f,         3,            Item{ItemType::item, 3, 1}},
   BlockInfo{"mythril_ore",        BlockType::solid,                                                                                          3.25f,        3,            Item{ItemType::item, 4, 1}},
};

const static inline StrArray<const char *> blockIds {
   "air", "grass", "dirt", "clay", "stone", "sand", "sandstone", "bricks", "glass", "planks", "stone_bricks", "tiles", "obsidian", "platform", "snow", "ice", "jungle_grass", "mud", "lamp", "torch",
   "honey_block", "crispy_honey_block", "slime_block", "bubble_block", "slime_platform", "cactus_block", "coal_ore", "iron_ore", "gold_ore", "mythril_ore"
};

// Block getter functions

float getBlockBreakingTime(unsigned short id) {
   assertDebug(id < blockCount, "DEBUG: Block with the id '{}' does not exist. Block count is {}.", id, blockCount);
   return blockInfo[id].breakTime;
}

int getBlockBreakingLevel(unsigned short id) {
   assertDebug(id < blockCount, "DEBUG: Block with the id '{}' does not exist. Block count is {}.", id, blockCount);
   return blockInfo[id].breakLevel;
}

// Asserts in these two functions would be too slow, as they're called often (especially in
// world generation code), that's why debug asserts are used instead
unsigned short getBlockIdFromName(const char *name) {
   assertDebug(blockIds.count(name), "DEBUG: Block with the name '{}' does not exist!", name);
   return blockIds.at(name);
}

std::string getBlockNameFromId(unsigned short id) {
   assertDebug(id < blockCount, "DEBUG: Block with the id '{}' does not exist. Block count is {}.", id, blockCount);
   return blockInfo[id].name;
}

Item getBlockDropId(unsigned short id, bool iswall) {
   return blockInfo[id].drop.id == 0 ? Item{ItemType::block, id, 1, false, iswall, false} : blockInfo.at(id).drop;
}

// Map constructors

void Map::init(bool thread) {
   timeShaderLocation = GetShaderLocation(getShader("water"), "time");
   lightmap = LoadRenderTexture(GetScreenWidth() / 2, GetScreenHeight() / 2);

   if (!thread) {
      initContainers();
   }
}

void Map::initContainers() {
   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
   walls  = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));

   liquidsHeights = std::vector<std::vector<unsigned char>>(sizeY, std::vector<unsigned char>(sizeX, 0));
   liquidTypes    = std::vector<std::vector<LiquidType>>(sizeY, std::vector<LiquidType>(sizeX, LiquidType::none));
}

Map::~Map() {
   UnloadRenderTexture(lightmap);
}

// Collision

bool Map::raycast(const Vector2 &start, const Vector2 &end) {
   Vector2 dir = Vector2Normalize({start.x - end.x, start.y - end.y});
   Vector2 stepSize = {fabs(1.0f / dir.x), fabs(1.0f / dir.y)};
   Vector2 mapcheck = {truncf(start.x), truncf(start.y)};
   Vector2 vstep;
   Vector2 length;

   if (dir.x < 0.0f) {
      vstep.x = -1.0f;
      length.x = (start.x - mapcheck.x) * stepSize.x;
   } else {
      vstep.x = 1.0f;
      length.x = ((mapcheck.x + 1.0f) - start.x) * stepSize.x;
   }

   if (dir.y < 0.0f) {
      vstep.y = -1.0f;
      length.y = (start.y - mapcheck.y) * stepSize.y;
   } else {
      vstep.y = 1.0f;
      length.y = ((mapcheck.y + 1.0f) - start.y) * stepSize.y;
   }

   float maxDist = Vector2Distance(start, end);
   float dist = 0.0f;

   while (dist < maxDist) {
      if (length.x < length.y) {
         mapcheck.x += vstep.x;
         dist = length.x;
         length.x += stepSize.x;
      } else {
         mapcheck.y += vstep.y;
         dist = length.y;
         length.y += stepSize.y;
      }

      if (isPositionValid(mapcheck.x, mapcheck.y)) {
         if (isu(mapcheck.x, mapcheck.y, BlockType::solid) && !isu(mapcheck.x, mapcheck.y, BlockType::platform)) {
            return true;
         }
      }
   }
   return false;
}

// Add damage indicator

void Map::addDamageIndicator(const Vector2 &position, int damage, bool critical) {
   damageIndicators.push_back({position, {random(-10.0f, 10.0f) * fixedUpdateDT, -5.0f * fixedUpdateDT}, 0.0f, damage, critical});
}

// Set block functions

void Map::setRow(int y, const char *name, bool isWall) {
   unsigned short id = getBlockIdFromName(name);
   BlockType type = blockInfo[id].attributes;

   (isWall ? walls : blocks)[y] = std::vector<Block>(sizeX, Block{&getTexture(name), type, id});
}

void Map::setRow(int y, unsigned short *ids) {
   for (int x = 0; x < sizeX; ++x) {
      Block &block = blocks[y][x];
      block.id = ids[x];
      block.type = blockInfo[block.id].attributes;

      if (block.id != 0) {
         block.texture = &getTexture(blockInfo[block.id].name);
      }
   }
}

void Map::setWallRow(int y, unsigned short *ids) {
   for (int x = 0; x < sizeX; ++x) {
      Block &block = walls[y][x];
      block.id = ids[x];
      block.type = blockInfo[block.id].attributes;

      if (block.id != 0) {
         block.texture = &getTexture(blockInfo[block.id].name);
      }
   }
}

void Map::setColumnAndWalls(int x, int y, const char *name) {
   unsigned short id = getBlockIdFromName(name);
   BlockType type = blockInfo[id].attributes;
   Texture *texture = &getTexture(name);
   
   for (int yy = y; yy < sizeY; ++yy) {
      Block &block = blocks[yy][x];
      block.id = id;
      block.type = type;
      block.texture = texture;

      Block &wall = walls[yy][x];
      wall.id = id;
      wall.texture = texture;
   }
}

void Map::setBlock(int x, int y, const char *name, bool isWall) {
   Block &block = (isWall ? walls : blocks)[y][x];
   block.id = getBlockIdFromName(name);
   block.value = block.value2 = 0;
   block.type = blockInfo[block.id].attributes | (block.type % (BlockType::furniture | BlockType::furnitureTop));

   if (!isWall && !(block.type & BlockType::flowable)) {
      liquidsHeights[y][x] = 0;
      liquidTypes[y][x] = LiquidType::none;
   }

   if (block.id != 0) {
      block.texture = &getTexture(name);
   }
}

void Map::setBlock(int x, int y, unsigned short id, bool isWall) {
   Block &block = (isWall ? walls : blocks)[y][x];
   block.id = id;
   block.value = block.value2 = 0;
   block.type = blockInfo[block.id].attributes | (block.type % (BlockType::furniture | BlockType::furnitureTop));

   if (!isWall && !(block.type & BlockType::flowable)) {
      liquidsHeights[y][x] = 0;
      liquidTypes[y][x] = LiquidType::none;
   }

   if (block.id != 0) {
      block.texture = &getTexture(blockInfo[id].name);
   }
}

void Map::lightSetBlock(int x, int y, unsigned short id) {
   Block &block = blocks[y][x];
   block.id = id;
   block.type = blockInfo[id].attributes;
   block.texture = &getTexture(blockInfo[id].name);
}

// Fuck furniture logic, rewrite it one day
void Map::deleteBlock(int x, int y, bool wall) {
   deleteBlockWithoutDeletingLiquids(x, y, wall);
   if (!wall) {
      liquidsHeights[y][x] = 0;
      liquidTypes[y][x] = LiquidType::none;
   }
}

void Map::deleteBlockWithoutDeletingLiquids(int x, int y, bool wall) {
   Block &block = (wall ? walls : blocks)[y][x];
   block.texture = nullptr;
   block.type = blockInfo[0].attributes | (block.type % (BlockType::furniture | BlockType::furnitureTop));
   block.id = block.value = block.value2 = 0;
}

// Fuck this
void Map::moveBlock(int oldX, int oldY, int newX, int newY) {
   constexpr BlockType furnitureMask = (BlockType::furniture | BlockType::furnitureTop);
   BlockType oldType = blocks[oldY][oldX].type % furnitureMask;
   BlockType newType = blocks[newY][newX].type % furnitureMask;

   std::swap(blocks[oldY][oldX], blocks[newY][newX]);
   std::swap(liquidsHeights[oldY][oldX], liquidsHeights[newY][newX]);
   std::swap(liquidTypes[oldY][oldX], liquidTypes[newY][newX]);

   blocks[newY][newX].type = (blocks[newY][newX].type % ~furnitureMask) | oldType;
   blocks[oldY][oldX].type = (blocks[oldY][oldX].type % ~furnitureMask) | newType;
}

// Set furniture functions

Furniture &Map::getFurnitureAtPosition(int x, int y) {
   for (Furniture &furniture: furniture) {
      if (furniture.posX <= x && furniture.posX + furniture.sizeX > x
         && furniture.posY <= y && furniture.posY + furniture.sizeY > y
         && !furniture.pieces[y - furniture.posY][x - furniture.posX].nil) {
         return furniture;
         break;
      }
   }
   assert(false, "Error: No furniture at position ({}, {}).", x, y);
   std::exit(-1); // Silence warning
}

void Map::addFurniture(Furniture &object) {
   for (int y = object.posY; y < object.sizeY + object.posY; ++y) {
      for (int x = object.posX; x < object.sizeX + object.posX; ++x) {
         if (object.pieces[y - object.posY][x - object.posX].nil) {
            continue;
         }
         blocks[y][x].type = blocks[y][x].type | BlockType::furniture;
         if (object.isWalkable && y == object.posY) {
            blocks[y][x].type = blocks[y][x].type | BlockType::furnitureTop;
         }
      }
   }
   furniture.push_back(object);
}

void Map::removeFurniture(Furniture &object) {
   for (int y = object.posY; y < object.sizeY + object.posY; ++y) {
      for (int x = object.posX; x < object.sizeX + object.posX; ++x) {
         if (!object.pieces[y - object.posY][x - object.posX].nil) {
            // Again, modulus is overloaded as 'bitwise and' here.
            blocks[y][x].type = blocks[y][x].type % ~(BlockType::furniture | BlockType::furnitureTop);
         }
      }
   }
   object.deleted = true;
}

// Get block functions

bool Map::isPositionValid(int x, int y) const {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::is(int x, int y, BlockType type) const {
   return isPositionValid(x, y) && (blocks[y][x].type & type);
}

bool Map::isu(int x, int y, BlockType type) const {
   return blocks[y][x].type & type;
}

bool Map::isSoil(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & (BlockType::grass | BlockType::dirt | BlockType::sand));
}

bool Map::isEmpty(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & BlockType::empty) && !isLiquid(x, y) && !(blocks[y][x].type & BlockType::furniture);
}

bool Map::isNotSolid(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & BlockType::empty) && !(blocks[y][x].type & BlockType::furniture);
}

bool Map::isStable(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & (BlockType::solid | BlockType::furniture)) && !(blocks[y][x].type & BlockType::platform);
}

bool Map::isPlatformedFurniture(int x, int y) const {
   return (blocks[y][x].type & BlockType::furniture) && (blocks[y][x].type & BlockType::furnitureTop);
}

// Liquid functions

bool Map::isLiquid(int x, int y) const {
   return isPositionValid(x, y) && liquidTypes[y][x] != LiquidType::none && liquidsHeights[y][x] > minLiquidLayers;
}

bool Map::isLiquidAtAll(int x, int y) const {
   return isPositionValid(x, y) && liquidTypes[y][x] != LiquidType::none;
}

unsigned char Map::getLiquidHeight(int x, int y) const {
   return liquidsHeights[y][x];
}

unsigned char Map::isLiquidOfType(int x, int y, LiquidType type) const {
   return liquidTypes[y][x] == type;
}

Texture &Map::getLiquidTexture(int x, int y) const {
   if (liquidTypes[y][x] == LiquidType::water) {
      return getTexture("water");
   } else if (liquidTypes[y][x] == LiquidType::lava) {
      return getTexture("lava");
   } else if (liquidTypes[y][x] == LiquidType::honey) {
      return getTexture("honey");
   } else {
      return getTexture(""); // Get fallback texture
   }
}

// Render functions

void Map::renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color) const {
   drawTexture(texture, {(((x + 0.5f - camera.target.x) * camera.zoom) + camera.offset.x) / 2.0f, (((y + 0.5f - camera.target.y) * camera.zoom) + camera.offset.y) / 2.0f}, size, 0, color);
}

void Map::render(const std::vector<DroppedItem> &droppedItems, const Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera, const Inventory &inventory) const {
   // Render background walls
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &wall = walls[y][x];
         if ((wall.type & BlockType::empty) || !(blocks[y][x].type & BlockType::transparent)) {
            continue;
         }

         int oldX = x;
         while (x <= cameraBounds.width && walls[y][x].id == wall.id && (blocks[y][x].type & BlockType::transparent)) {
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
         const Block &block = blocks[y][x];
         if (block.type & BlockType::empty) {
            continue;
         }

         // Render torches
         else if (block.type & BlockType::torch) {
            constexpr static float torchLightOffsetsY[] = {-1.0f, -1.0f * (5.0f / 8.0f), -0.75f, -0.75f, -1.0f * (5.0f / 8.0f)};

            float textureSize = block.texture->height / 2.0f;
            DrawTexturePro(*block.texture, {textureSize * block.value2, 0, textureSize, textureSize}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
            DrawTexturePro(*block.texture, {textureSize * block.value, textureSize, textureSize, textureSize}, {(float)x, (float)y + torchLightOffsetsY[block.value2], 1, 1}, {0, 0}, 0, WHITE);
            continue;
         }

         // Render regular blocks
         int oldX = x;
         while (x <= cameraBounds.width && blocks[y][x].id == block.id) {
            x += 1;
         }

         drawTextureBlock(*block.texture, {(float)oldX, (float)y, float(x - oldX), 1});
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
   SetShaderValue(waterShader, timeShaderLocation, &time, SHADER_UNIFORM_FLOAT);

   // Render fluids
   BeginShaderMode(waterShader);
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         if (!isLiquid(x, y)) {
            continue;
         }
         
         float height = (float)getLiquidHeight(x, y) / (float)maxLiquidLayers;
         Color liquidFlags;
         liquidFlags.r = (is(x, y - 1, BlockType::solid) && !is(x, y - 1, BlockType::platform) ? 255 : 0);
         liquidFlags.g = (!isLiquidAtAll(x, y + 1) || !isLiquidOfType(x, y + 1, liquidTypes[y][x]) ? 255 : 0);

         drawFluidBlock(getLiquidTexture(x, y), {(float)x, (float)y + (1 - height), 1, height}, Fade(liquidFlags, height));
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
         BlockType type = blocks[y][x].type;
         if (!(type & BlockType::lightsource) && !(type & BlockType::transparent)) {
            continue;
         }

         // Direct light sources, ones who do not require the wall behind to be transparent
         if (isLiquidAtAll(x, y) && isLiquidOfType(x, y, LiquidType::lava)) {
            if (isLiquid(x, y)) {
               renderLight(camera, lightTexture, x + positionOffset, y + positionOffset, liquidSize, {255, 125, 0, 255});
            }
            continue;
         } else if (type & BlockType::torch) {
            renderLight(camera, lightHugeTexture, x + positionOffset, y + positionOffset, lightHugeSize, {255, 200, 160, 255}); // Light orange
         } else if (type & BlockType::lightsource) {
            renderLight(camera, lightLargeTexture, x, y, lightLargeSize, {255, 255, 0, 255});
         }

         if (!(walls[y][x].type & BlockType::transparent)) {
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
