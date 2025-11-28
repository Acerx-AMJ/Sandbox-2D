#ifndef OBJS_FURNITURE_HPP
#define OBJS_FURNITURE_HPP

#include <raylib.h>
#include <string>
#include <vector>

// Furniture

struct FurniturePiece {
   unsigned char tx = 0, ty = 0;
   unsigned char colorId = 0;
   bool nil = true;
};

struct Furniture {
   enum Type { tree };
   using id_t = unsigned char;

   std::vector<std::vector<FurniturePiece>> pieces;
   Type type = Type::tree;
   id_t texId = 0;

   unsigned char value = 0, value2 = 0;
   int posX = 0, posY = 0, sizeX = 0, sizeY = 0;

   // Constructors

   Furniture() = default;
   Furniture(Type type, id_t texId, unsigned char value, unsigned char value2, int posX, int posY, int sizeX, int sizeY);
   Furniture(const std::string& texture, int posX, int posY, int sizeX, int sizeY, Type type);

   // Render functions

   void render(bool zoomedOut, int minX, int minY, int maxX, int maxY);

   // Getter functions

   static id_t getId(const std::string& name);
   static std::string getName(id_t id);
};

// Furniture methods

struct Map;
void generateTree(int x, int y, Map& map);
void renderTree(Furniture& tree);

#endif
