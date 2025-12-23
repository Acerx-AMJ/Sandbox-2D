#ifndef OBJS_FURNITURE_HPP
#define OBJS_FURNITURE_HPP

#include <raylib.h>
#include <string>
#include <vector>
struct Map;

// Furniture

struct FurnitureTexture {
   Texture &texture;
   float sizeX = 0, sizeY = 0;
};

struct FurniturePiece {
   unsigned char tx = 0, ty = 0;
   bool nil = true;
};

struct Furniture {
   enum Type { none, tree, sapling, cactus, cactus_seed };

   std::vector<std::vector<FurniturePiece>> pieces;
   Type type = Type::tree;
   unsigned char texId = 0;

   int value = 0, value2 = 0;
   int posX = 0, posY = 0, sizeX = 0, sizeY = 0;
   bool deleted = false;

   // Constructors

   Furniture() = default;
   Furniture(Type type, unsigned char texId, int value, int value2, int posX, int posY, int sizeX, int sizeY);
   Furniture(const std::string &texture, int posX, int posY, int sizeX, int sizeY, Type type);

   // Update functions

   void update(Map &map);

   // Getter functions

   static Furniture get(int x, int y, const Map &map, Type type, bool debug = false);
   static void generate(int x, int y, Map &map, Type type);

   // Render functions

   void preview(const Map &map) const;
   void render(const Rectangle &cameraBounds) const;

   // Id functions

   static unsigned char getId(const std::string &name);
   static std::string getName(unsigned char id);
   static FurnitureTexture getFurnitureIcon(unsigned char id);
};

#endif
