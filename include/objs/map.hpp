#ifndef OBJS_MAP_HPP
#define OBJS_MAP_HPP

#include "objs/furniture.hpp"
#include <string>

// Constants

constexpr inline Color wallTint = {120, 120, 120, 255};

constexpr inline unsigned char maxWaterLayers     = 32;
constexpr inline unsigned char minWaterLayers     = maxWaterLayers / 8;
constexpr inline unsigned char lavaLayerThreshold = maxWaterLayers / 4;
constexpr inline unsigned char playerThreshold    = maxWaterLayers / 2;

// Block

// Bad practice to make enums unsigned, but as blocks attributes will grow, it'll be
// increasingly useful, also makes bitwise operations slightly easier (probably).
enum class BlockType: unsigned short {
   empty        = 1 <<  0,
   grass        = 1 <<  1,
   dirt         = 1 <<  2,
   sand         = 1 <<  3,
   ice          = 1 <<  4,
   solid        = 1 <<  5,
   platform     = 1 <<  6,
   transparent  = 1 <<  7,
   water        = 1 <<  8,
   lava         = 1 <<  9,
   liquid       = 1 << 10,
   lightsource  = 1 << 11,
   torch        = 1 << 12,
   furniture    = 1 << 13,
   furnitureTop = 1 << 14,
};

// Block type operators
constexpr inline BlockType operator|(BlockType lhs, BlockType rhs) {
   return static_cast<BlockType>(static_cast<unsigned short>(lhs) | static_cast<unsigned short>(rhs));
}

constexpr inline bool operator&(BlockType lhs, BlockType rhs) {
   return (static_cast<unsigned short>(lhs) & static_cast<unsigned short>(rhs)) != 0;
}

// Instead of retrieving a boolean like the previous function, return the result as
// a block type. DOES NOT ACT LIKE A MODULUS OPERATOR! 
constexpr inline BlockType operator%(BlockType lhs, BlockType rhs) {
   return static_cast<BlockType>(static_cast<unsigned short>(lhs) & static_cast<unsigned short>(rhs));
}

constexpr inline BlockType operator~(BlockType bt) {
   return static_cast<BlockType>(~static_cast<unsigned short>(bt));
}

// Currently has 2 free bytes due to struct padding, meaning one short or two chars
// can be safely added without expanding memory usage
struct Block {
   Texture *texture = nullptr;
   BlockType type = BlockType::empty | BlockType::transparent; // air
   unsigned short id = 0;

   // Values used by physics updates, specific to the block type
   unsigned char value = 0;
   unsigned char value2 = 0;
};

// Block getter functions
unsigned short getBlockIdFromName(const std::string &name);
std::string getBlockNameFromId(unsigned short id);

// Map

struct Map {   
   // Constructors

   void init();
   ~Map();

   // Set block functions

   void setRow(int y, const std::string &name, bool isWall = false);
   void setRow(int y, unsigned short *ids, unsigned char *physicsValues);
   void setWallRow(int y, unsigned short *ids);
   
   void setBlock(int x, int y, const std::string &name, bool isWall = false);
   void setBlock(int x, int y, unsigned short id, bool isWall = false);

   void deleteBlock(int x, int y, bool isWall = false);
   void moveBlock(int oldX, int oldY, int newX, int newY);

   // Set furniture functions

   void addFurniture(Furniture &object);
   void removeFurniture(Furniture &object);

   // Get block functions

   bool isPositionValid(int x, int y) const;
   bool is(int x, int y, BlockType type) const;
   bool isu(int x, int y, BlockType type) const; // Unsafe is variant

   bool isSoil(int x, int y) const;
   bool isLiquid(int x, int y) const;
   bool isEmpty(int x, int y) const;
   bool isNotSolid(int x, int y) const; // Is either empty or a full liquid
   bool isStable(int x, int y) const; // Is solid or furniture and not platform
   bool isPlatformedFurniture(int x, int y) const;

   // Render map

   void renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color) const;
   void render(const std::vector<struct DroppedItem> &droppedItems, const struct Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera) const;

   // Members

   RenderTexture lightmap;
   std::vector<std::vector<Block>> blocks, walls;
   std::vector<Furniture> furniture;

   int sizeX = 0;
   int sizeY = 0;
   int timeShaderLocation = 0;
};

#endif
