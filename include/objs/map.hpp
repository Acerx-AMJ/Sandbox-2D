#ifndef OBJS_MAP_HPP
#define OBJS_MAP_HPP

#include "objs/furniture.hpp"
#include <string>

// Constants

constexpr inline Color wallTint = {120, 120, 120, 255};

constexpr inline unsigned char maxLiquidLayers         = 32;
constexpr inline unsigned char minLiquidLayers         = maxLiquidLayers / 8;
constexpr inline unsigned char liquidToBlockThreshold  = maxLiquidLayers / 4;
constexpr inline unsigned char playerLiquidThreshold   = maxLiquidLayers / 2;
constexpr inline float         damageIndicatorLifetime = 1.0f;

// Block

// Liquid enum
enum class LiquidType: unsigned char {
   none,
   water,
   lava,
   honey,
};

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
   lightsource  = 1 <<  8,
   torch        = 1 <<  9,
   furniture    = 1 << 10,
   furnitureTop = 1 << 11,
   flowable     = 1 << 12,
   sticky       = 1 << 13,
   bouncy       = 1 << 14,
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
   BlockType type = BlockType::empty | BlockType::transparent | BlockType::flowable; // air
   unsigned short id = 0;

   // Values used by physics updates, specific to the block type
   unsigned char value = 0;
   unsigned char value2 = 0;
};

// Block getter functions
float getBlockBreakingTime(unsigned short id);
int getBlockBreakingLevel(unsigned short id);
unsigned short getBlockIdFromName(const std::string &name);
std::string getBlockNameFromId(unsigned short id);

// Damage indicator

struct DamageIndicator {
   Vector2 position, velocity;
   float lifetime = 0.0f;
   int damage = 0;
   bool critical = false;
};

// Map

struct Map {   
   // Constructors

   void init(bool thread = false);
   void initContainers();
   ~Map();

   // Collision checks

   bool raycast(const Vector2 &start, const Vector2 &end);

   // Add damage indicator

   void addDamageIndicator(const Vector2 &position, int damage, bool critical);

   // Set block functions

   void setRow(int y, const std::string &name, bool isWall = false);
   void setRow(int y, unsigned short *ids);
   void setWallRow(int y, unsigned short *ids);
   void setColumnAndWalls(int x, int y, const std::string &name);
   
   void setBlock(int x, int y, const std::string &name, bool isWall = false);
   void setBlock(int x, int y, unsigned short id, bool isWall = false);
   void lightSetBlock(int x, int y, unsigned short id);

   void deleteBlock(int x, int y, bool isWall = false);
   void deleteBlockWithoutDeletingLiquids(int x, int y, bool isWall = false);
   void moveBlock(int oldX, int oldY, int newX, int newY);

   // Set furniture functions

   Furniture &getFurnitureAtPosition(int x, int y);
   void addFurniture(Furniture &object);
   void removeFurniture(Furniture &object);

   // Get block functions

   bool isPositionValid(int x, int y) const;
   bool is(int x, int y, BlockType type) const;
   bool isu(int x, int y, BlockType type) const; // Unsafe is variant

   bool isSoil(int x, int y) const;
   bool isEmpty(int x, int y) const;
   bool isNotSolid(int x, int y) const; // Is either empty or a full liquid
   bool isStable(int x, int y) const; // Is solid or furniture and not platform
   bool isPlatformedFurniture(int x, int y) const;

   // Liquid functions

   bool isLiquid(int x, int y) const;
   bool isLiquidAtAll(int x, int y) const;
   unsigned char getLiquidHeight(int x, int y) const;
   unsigned char isLiquidOfType(int x, int y, LiquidType type) const;
   Texture &getLiquidTexture(int x, int y) const;

   // Render map

   void renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color) const;
   void render(const std::vector<struct DroppedItem> &droppedItems, const struct Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera, const Inventory &inventory) const;

   // Members

   RenderTexture lightmap;
   std::vector<std::vector<Block>> blocks, walls;
   std::vector<Furniture> furniture;

   std::vector<std::vector<unsigned char>> liquidsHeights;
   std::vector<std::vector<LiquidType>>    liquidTypes;

   std::vector<DamageIndicator> damageIndicators;

   int sizeX = 0;
   int sizeY = 0;
   int timeShaderLocation = 0;
};

#endif
