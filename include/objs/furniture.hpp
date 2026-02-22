#ifndef OBJS_FURNITURE_HPP
#define OBJS_FURNITURE_HPP

#include <raylib.h>
#include <string>
#include <vector>

// Constants

constexpr inline float previewAlpha = 0.75f;

// Furniture

// Polymorphism would be too complex in this situation and everything
// works fine just as it is. Also when adding new furniture, you have
// to change the following: getFurnitureIcon, getFurniture, Furniture
// ::isValid, Furniture::update and furniture ID constants
enum class FurnitureType: unsigned char {
   none,
   tree,
   sapling,
   cactus,
   cactusSeed,
   table,
   chair,
   door
};

struct FurnitureTexture {
   Texture &texture;

   // Floats just to avoid redundant casting
   float sizeX = 0;
   float sizeY = 0;
};

struct FurniturePiece {
   unsigned char tx = 0;
   unsigned char ty = 0;
   bool nil = true;
};

struct Furniture {
   // Constructors

   Furniture() = default;
   Furniture(FurnitureType type, unsigned short id, short value, short value2, int posX, int posY, short sizeX, short sizeY);
   Furniture(const std::string &texture, int posX, int posY, short sizeX, short sizeY, FurnitureType type);

   // Update functions

   void destroy(struct Map &map, struct Inventory &inventory, int cursorX, int cursorY, int breakingLevel);
   void update(struct Map &map, struct Player &player, const Vector2 &mousePos);
   bool isValid(const struct Map &map) const;

   // Render functions

   void preview(const struct Map &map) const;
   void render(const Rectangle &cameraBounds) const;

   // Members

   std::vector<std::vector<FurniturePiece>> pieces;
   FurnitureType type = FurnitureType::none;
   unsigned short id = 0;

   short value = 0;
   short value2 = 0;
   int posX = 0;
   int posY = 0;
   short sizeX = 0;
   short sizeY = 0;

   bool deleted = false;
   bool isWalkable = false;
};

// Furniture getter functions

float getFurnitureBreakingTime(unsigned short id);
unsigned short getFurnitureIdFromName(const std::string &name);
std::string getFurnitureNameFromId(unsigned short id);
FurnitureType getFurnitureType(unsigned short id);
FurnitureTexture getFurnitureIcon(unsigned short id);

// Furniture generation functions

Furniture getFurniture(int x, int y, const struct Map &map, FurnitureType type, bool playerFacingLeft, bool debug = false);
void generateFurniture(int x, int y, struct Map &map, FurnitureType type, bool playerFacingLeft);

#endif
