#ifndef OBJS_FURNITURE_HPP
#define OBJS_FURNITURE_HPP

#include <raylib.h>
#include <string>
#include <vector>
struct Map;

// Furniture

struct FurniturePiece {
   unsigned char tx = 0, ty = 0;
   bool nil = true;
};

struct Furniture {
   enum Type { none, tree, sapling, cactus, cactus_seed };
   using id_t = unsigned char;

   std::vector<std::vector<FurniturePiece>> pieces;
   Type type = Type::tree;
   id_t texId = 0;

   int value = 0, value2 = 0;
   int posX = 0, posY = 0, sizeX = 0, sizeY = 0;
   bool deleted = false;

   // Constructors

   Furniture() = default;
   Furniture(Type type, id_t texId, int value, int value2, int posX, int posY, int sizeX, int sizeY);
   Furniture(const std::string &texture, int posX, int posY, int sizeX, int sizeY, Type type);

   // Update functions

   void update(Map &map);

   // Getter functions

   static Furniture get(int x, int y, Map &map, Type type, bool debug = false);
   static void generate(int x, int y, Map &map, Type type);

   // Render functions

   void preview(Map &map);
   void render(int minX, int minY, int maxX, int maxY);

   // Id functions

   static id_t getId(const std::string &name);
   static std::string getName(id_t id);
};

#endif
