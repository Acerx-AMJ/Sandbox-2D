#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "objs/item.hpp"
#include "objs/player.hpp"
#include "util/format.hpp"
#include "util/parallax.hpp"
#include "util/render.hpp"
#include <array>
#include <cmath>
#include <unordered_map>

// Constants

constexpr unsigned char blockCount = 22;

static inline const std::unordered_map<std::string, unsigned char> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4},
   {"sand", 5}, {"sandstone", 6}, {"water", 7}, {"bricks", 8}, {"glass", 9},
   {"planks", 10}, {"stone_bricks", 11}, {"tiles", 12}, {"obsidian", 13}, {"lava", 14},
   {"platform", 15}, {"snow", 16}, {"ice", 17}, {"mud", 18}, {"jungle_grass", 19},
   {"lamp", 20}, {"torch", 21}
};

static inline const std::array<const char*, blockCount> blockNames {
   "air", "grass", "dirt", "clay", "stone",
   "sand", "sandstone", "water", "bricks", "glass",
   "planks", "stone_bricks", "tiles", "obsidian", "lava",
   "platform", "snow", "ice", "mud", "jungle_grass",
   "lamp", "torch"
};

static inline const std::array<Block::Type, blockCount> blockTypes {{
   Block::air, Block::grass, Block::dirt, Block::solid, Block::solid,
   Block::sand, Block::solid, Block::water, Block::solid, Block::transparent,
   Block::solid, Block::solid, Block::solid, Block::solid, Block::lava,
   Block::platform, Block::snow, Block::ice, Block::dirt, Block::grass,
   Block::lamp, Block::torch
}};

// Block functions

unsigned char Block::getId(const std::string &name) {
   assertDebug(blockIds.count(name), "DEBUG: Block with the name '{}' does not exist!", name);
   return blockIds.at(name);
}

std::string Block::getName(unsigned char id) {
   assertDebug(id < blockCount, "DEBUG: Block with the id '{}' does not exist. Valid IDs are in range {} to {}.", (int)id, 0, (int)blockCount - 1);
   return blockNames.at(id);
}

// Map destructors

Map::~Map() {
   UnloadRenderTexture(lightmap);
}

// Set block functions

void Map::init() {
   lightmap = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
   walls  = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
}

void Map::setRow(int y, const std::string &name, bool isWall) {
   unsigned char id = Block::getId(name);
   Block::Type type = blockTypes[id];

   (isWall ? walls : blocks)[y] = std::vector<Block>(sizeX, Block{&getTexture(name), type, id, false, 0, (type == Block::water || type == Block::lava ? maxWaterLayers : (unsigned char)0)});
}

void Map::setRow(int y, unsigned char *ids, unsigned char *physicsValues) {
   for (int x = 0; x < sizeX; ++x) {
      Block &block = blocks[y][x];
      block.id = ids[x];
      block.value = 0;
      block.value2 = physicsValues[x];
      block.type = blockTypes[block.id];

      if (block.id != 0) {
         block.texture = &getTexture(blockNames[block.id]);
      }
   }
}

void Map::setWallRow(int y, unsigned char *ids) {
   for (int x = 0; x < sizeX; ++x) {
      Block &block = walls[y][x];
      block.id = ids[x];
      block.value = block.value2 = 0;
      block.type = blockTypes[block.id];

      if (block.id != 0) {
         block.texture = &getTexture(blockNames[block.id]);
      }
   }
}

void Map::setBlock(int x, int y, const std::string &name, bool isWall) {
   Block &block = (isWall ? walls : blocks)[y][x];
   
   block.id = Block::getId(name);
   block.value = block.value2 = 0;
   block.type = blockTypes[block.id];

   if (block.id != 0) {
      block.texture = &getTexture(name);
   }

   if (block.type == Block::water || block.type == Block::lava) {
      block.value2 = maxWaterLayers;
   }
}

void Map::setBlock(int x, int y, unsigned char id, bool isWall) {
   setBlock(x, y, Block::getName(id), isWall);
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

bool Map::isPositionValid(int x, int y) const {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::is(int x, int y, Block::Type type) const {
   return isPositionValid(x, y) && blocks[y][x].type == type;
}

bool Map::isu(int x, int y, Block::Type type) const {
   return blocks[y][x].type == type;
}

bool Map::empty(int x, int y) const {
   return isPositionValid(x, y) && !blocks[y][x].furniture && blocks[y][x].type == Block::air;
}

bool Map::isTransparent(int x, int y) const {
   Block::Type type = blocks[y][x].type;
   return type == Block::air || type == Block::water || type == Block::transparent || type == Block::platform || type == Block::torch;
}

std::vector<Block>& Map::operator[](size_t index) {
   return blocks[index];
}

// Render functions

void Map::renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color) const {
   drawTexture(texture, GetWorldToScreen2D({x + 0.5f, y + 0.5f}, camera), size, 0, color);
}

void Map::render(const std::vector<DroppedItem> &droppedItems, const Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera) const {
   // Render background walls
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &wall = walls[y][x];
         if (wall.type == Block::air || !isTransparent(x, y)) {
            continue;
         }

         int oldX = x;
         while (x <= cameraBounds.width && walls[y][x].id == wall.id && isTransparent(x, y)) {
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

   // Render furniture-like blocks before the player
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &block = blocks[y][x];
         if (block.type == Block::platform) {
            DrawTexturePro(*block.texture, {0, 0, (float)block.texture->width, (float)block.texture->height}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
         } else if (block.type == Block::torch) {
            constexpr static float torchLightOffsetsY[] = {-1.0f, -1.0f * (5.0f / 8.0f), -0.75f, -0.75f, -1.0f * (5.0f / 8.0f)};

            float textureSize = block.texture->height / 2.0f;
            DrawTexturePro(*block.texture, {textureSize * block.value2, 0, textureSize, textureSize}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
            DrawTexturePro(*block.texture, {textureSize * block.value, textureSize, textureSize, textureSize}, {(float)x, (float)y + torchLightOffsetsY[block.value2], 1, 1}, {0, 0}, 0, WHITE);
         }
      }
   }

   // Render the player
   player.render(accumulator);

   for (const DroppedItem &droppedItem : droppedItems) {
      droppedItem.render();
   }

   // Render blocks
   Shader &waterShader = getShader("water");

   int timeLocation = GetShaderLocation(waterShader, "time");
   float time = GetTime();
   SetShaderValue(waterShader, timeLocation, &time, SHADER_UNIFORM_FLOAT);

   struct Liquid {
      const Block *pointer;
      int x, y;
   };
   std::vector<Liquid> liquidTiles;
   liquidTiles.reserve(64);

   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &block = blocks[y][x];
         if (block.type == Block::air || block.type == Block::platform || block.type == Block::torch) {
            continue;
         }

         // Render fluids
         if (block.type == Block::water || block.type == Block::lava) {
            if (block.value2 <= minWaterLayers) {
               continue;
            }
            liquidTiles.push_back(Liquid{&block, x, y});
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

   // Render fluids
   BeginShaderMode(waterShader);
   for (const Liquid &block: liquidTiles) {
      float height = (float)block.pointer->value2 / (float)maxWaterLayers;
      Color liquidFlags;
      liquidFlags.r = (!is(block.x, block.y - 1, block.pointer->type) && !is(block.x, block.y - 1, Block::air) ? 255 : 0);
      liquidFlags.g = (!is(block.x, block.y + 1, block.pointer->type) ? 255 : 0);

      drawFluidBlock(*block.pointer->texture, {(float)block.x, (float)block.y + (1 - height), 1, height}, Fade(liquidFlags, height));
   }
   EndShaderMode();

   // // Render lights
   BeginTextureMode(lightmap);
   ClearBackground(BLACK);
   BeginBlendMode(BLEND_ADDITIVE);

   int lightBoundsMinX = std::max<int>(0, cameraBounds.x - 6);
   int lightBoundsMinY = std::max<int>(0, cameraBounds.y - 6);
   int lightBoundsMaxX = std::min<int>(sizeX - 1, cameraBounds.width + 6);
   int lightBoundsMaxY = std::min<int>(sizeY - 1, cameraBounds.height + 6);

   Color airLightColor   = getLightBasedOnTime();
   Color waterLightColor = Fade(airLightColor, 0.1f);

   // const function hack
   static float counter = 0.0f;
   counter += GetFrameTime();

   float sizeOffset     = std::sin(counter * 1.5f) * camera.zoom * 0.8f;
   float positionOffset = std::cos(counter * 0.8f) * camera.zoom * 0.015f;

   Vector2 lightSize      = {7.0f * camera.zoom, 7.0f * camera.zoom};
   Vector2 lightLargeSize = {lightSize.x + lightSize.x, lightSize.y + lightSize.y};
   Vector2 lightHugeSize  = {lightLargeSize.x + lightSize.x, lightLargeSize.y + lightSize.y};
   Vector2 liquidSize     = {lightSize.x + sizeOffset, lightSize.y + sizeOffset};

   Texture2D &lightHugeTexture  = getTexture("lightsource_6x");
   Texture2D &lightLargeTexture = getTexture("lightsource_4x");
   Texture2D &lightTexture      = getTexture("lightsource_2x");

   for (int y = lightBoundsMinY; y <= lightBoundsMaxY; ++y) {
      for (int x = lightBoundsMinX; x <= lightBoundsMaxX; ++x) {
         if (isTransparent(x, y)) {
            switch (walls[y][x].type) {
            case Block::lamp:
               renderLight(camera, lightLargeTexture, x, y, lightLargeSize, {255, 255, 0, 255});
               break;
            default: break;
            }
         }

         switch (blocks[y][x].type) {
         case Block::lava:
            renderLight(camera, lightTexture, x + positionOffset, y + positionOffset, liquidSize, {255, 125, 0, 255});
            break;
         case Block::lamp:
            renderLight(camera, lightLargeTexture, x, y, lightLargeSize, {255, 255, 0, 255});
            break;
         case Block::torch:
            renderLight(camera, lightHugeTexture, x + positionOffset, y + positionOffset, lightHugeSize, {255, 200, 160, 255}); // Light orange
            break;
         case Block::water:
            if (walls[y][x].type == Block::transparent || walls[y][x].type == Block::air) {
               renderLight(camera, lightTexture, x, y, lightSize, waterLightColor);
            }
            break;
         case Block::air:
         case Block::transparent:
         case Block::platform:
            if (walls[y][x].type == Block::transparent || walls[y][x].type == Block::air) {
               renderLight(camera, lightTexture, x, y, lightSize, airLightColor);
            }
            break;
         default: break;
         }
      }
   }

   EndBlendMode();
   EndTextureMode();

   BeginBlendMode(BLEND_MULTIPLIED);
   DrawTexturePro(lightmap.texture, {0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()}, {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, {0, 0}, 0, WHITE);
   EndBlendMode();

   // DrawFPS(100, 100);
   BeginMode2D(camera); // EndTextureMode disables it for some reason
}
