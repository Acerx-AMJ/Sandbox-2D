#pragma once
#include "objs/furniture.hpp"
#include <unordered_map>

// constants

constexpr inline Color wallTint = {120, 120, 120, 255};
constexpr inline liquidlayer_t maxLiquidLayers = 32;
constexpr inline liquidlayer_t minLiquidLayers = maxLiquidLayers / 8;
constexpr inline liquidlayer_t liquidToBlockThreshold = maxLiquidLayers / 8;
constexpr inline liquidlayer_t playerLiquidThreshold = maxLiquidLayers / 2;

// block

enum class TileType: unsigned char {
   root,
   ghost,
};

enum class BlockType: unsigned short {
   empty        = 1 <<  0,
   grass        = 1 <<  1,
   dirt         = 1 <<  2,
   sand         = 1 <<  3,
   ice          = 1 <<  4,
   solid        = 1 <<  5,
   platform     = 1 <<  6,
   translucent  = 1 <<  7,
   lightsource  = 1 <<  8,
   torch        = 1 <<  9,
   flowable     = 1 << 10,
   sticky       = 1 << 11,
   bouncy       = 1 << 12,
};

constexpr inline BlockType operator | (BlockType lhs, BlockType rhs) {
   return static_cast<BlockType>(static_cast<unsigned short>(lhs) | static_cast<unsigned short>(rhs));
}

constexpr inline BlockType operator & (BlockType lhs, BlockType rhs) {
   return static_cast<BlockType>(static_cast<unsigned short>(lhs) & static_cast<unsigned short>(rhs));
}

constexpr inline bool BlockTypeHas(BlockType lhs, BlockType rhs) {
   return (static_cast<unsigned short>(lhs) & static_cast<unsigned short>(rhs)) != 0;
}

struct Block {
   blockid_t id = 0;
   TileType tile = TileType::root;
   BlockType type = BlockType::empty | BlockType::translucent | BlockType::flowable;

   // Values used by physics updates, specific to the block type
   unsigned short value = 0;
   unsigned short value2 = 0;
   bool platformOverride = false; // platformed furniture
};

struct Wall {
   blockid_t id = 0;
   BlockType type = BlockType::empty | BlockType::translucent | BlockType::flowable;
};

bool isBlockTypeValid(const std::string &name);
bool isBlockNameValid(const std::string &name);
bool isBlockIdValid(blockid_t id);
BlockType getBlockTypeFromString(const std::string &name);
blockid_t getBlockIdFromName(const std::string &name);
std::string getBlockNameFromId(blockid_t id);
size_t getBlockCount();

void reserveBlockContainers(size_t estimate);
void pushBlock(const std::string &name);
void setBlock(const std::string &name, BlockType attributes, Texture texture);

// Liquids

struct LiquidData {
   Texture texture {0};
   int updateSpeed = 0;
   std::unordered_map<liquidid_t, blockid_t> conversionTable;
   float moveSpeedMultiplier = 1.0f;
   bool naturalLight = false;
   bool glow = false;
   bool damagePlayer = false;
   int damageMin = 0;
   int damageMax = 0;
};

bool isLiquidNameValid(const std::string &name);
bool isLiquidIdValid(liquidid_t id);
LiquidData &getLiquidData(liquidid_t id);
liquidid_t getLiquidIdFromName(const std::string &name);
std::string getLiquidNameFromId(liquidid_t id);
size_t getLiquidCount();

void reserveLiquidContainers(size_t estimate);
void pushLiquid(const std::string &name);
void setLiquid(const std::string &name, const LiquidData &data);

// Map

struct Map {   
   void init();
   void initThreadSafe();
   void initContainers();
   ~Map();

   // setters

   void setRow(int y, const std::string &name);
   void setWallRow(int y, const std::string &name);
   void setRow(int y, blockid_t *ids);
   void setWallRow(int y, blockid_t *ids);
   void setColumnFromPoint(int x, int y, const std::string &name);

   void setBlock(int x, int y, const std::string &name);
   void setBlock(int x, int y, blockid_t id);
   void setWall(int x, int y, const std::string &name);
   void setWall(int x, int y, blockid_t id);
   void setLiquid(int x, int y, liquidid_t id, liquidlayer_t height);

   void deleteBlock(int x, int y);
   void deleteWall(int x, int y);
   void deleteBlockWithoutDeletingLiquids(int x, int y);
   void swapBlocks(int oldX, int oldY, int newX, int newY);

   void addFurniture(Furniture &object);
   void removeFurniture(Furniture &object);

   // getters

   const Block &getBlock(int x, int y) const;
   const Wall &getWall(int x, int y) const;

   bool isPositionValid(int x, int y) const;
   bool isPositionValid(Vector2 position) const;

   bool is(int x, int y, BlockType type) const;
   bool isWall(int x, int y, BlockType type) const;
   bool isSoil(int x, int y) const;
   bool isEmpty(int x, int y) const;
   bool isNotSolid(int x, int y) const;
   bool isStable(int x, int y) const;

   bool isLiquid(int x, int y) const;
   bool isAnyLiquid(int x, int y) const;
   bool isLiquidOfType(int x, int y, liquidid_t id) const;
   liquidlayer_t getLiquidHeight(int x, int y) const;
   liquidid_t getLiquidId(int x, int y) const;
   LiquidData &getLiquidData(int x, int y) const;

   // render

   void renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color);
   void render(const std::vector<struct DroppedItem> &droppedItems, const struct Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera, const struct Inventory &inventory);

   // Members

   RenderTexture lightmap;
   std::vector<Block> blocks;
   std::vector<Wall> walls;
   std::vector<liquidlayer_t> liquidHeights;
   std::vector<liquidid_t> liquidTypes;
   std::vector<Furniture> furniture;

   int sizeX = 0;
   int sizeY = 0;
   int waterTimeShaderLocation = 0;
};
