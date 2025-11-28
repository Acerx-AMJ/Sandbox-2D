#include "objs/furniture.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "objs/map.hpp"
#include "util/random.hpp"
#include <array>
#include <unordered_map>

// Constants

constexpr int furnitureCount = 1;
static std::unordered_map<std::string, int> furnitureTextureIds {
   {"tree", 0}
};

constexpr std::array<const char*, furnitureCount> furnitureTextureNames {
   "tree"
};

constexpr int texSize = 8;

// Helper functions

void setBlock(FurniturePiece& piece, const std::string& id, int tx, int ty) {
   piece.nil = false;
   piece.colorId = Block::getId(id);
   piece.tx = tx;
   piece.ty = ty;
}

bool isBlockSoil(Map& map, int x, int y) {
   return map.is(x, y, Block::grass) or map.isu(x, y, Block::dirt);
}

// Furniture functions

Furniture::Furniture(Type type, id_t texId, unsigned char value, unsigned char value2, int posX, int posY, int sizeX, int sizeY)
   : type(type), texId(texId), value(value), value2(value2), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));   
}

Furniture::Furniture(const std::string& texture, int posX, int posY, int sizeX, int sizeY, Type type)
   : texId(furnitureTextureIds[texture]), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY), type(type) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));
}

void Furniture::render(bool zoomedOut, int minX, int minY, int maxX, int maxY) {
   for (int y = posY; y < maxY and y - posY < sizeY; ++y) {
      for (int x = posX; x < maxX and x - posX < sizeX; ++x) {
         auto& piece = pieces[y - posY][x - posX];
         if (y < minY or x < minX or piece.nil) {
            continue;
         }

         if (zoomedOut) {
            DrawRectangle(x, y, 1, 1, Block::getColorFromId(piece.colorId));
         } else {
            DrawTexturePro(getTexture(furnitureTextureNames[texId]), {(float)piece.tx, (float)piece.ty, texSize, texSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, WHITE);
         }
      }
   }
}

// Get functions

Furniture::id_t Furniture::getId(const std::string& name) {
   return furnitureTextureIds[name];
}

std::string Furniture::getName(id_t id) {
   return furnitureTextureNames[id];
}

// Furniture methods

void generateTree(int x, int y, Map& map) {
   if (x < 1 or x >= map.sizeX - 1 or y < 0 or not isBlockSoil(map, x , y + 1)) {
      return;
   }
   
   int height = random(5, 18);
   for (int i = 0; i < height and i < map.sizeY; ++i) {
      if (not map.isu(x, y - i, Block::air)) {
         height = i + 1;
         break;
      }
   }

   if (height < 5) {
      return;
   }
   Furniture tree ("tree", x - 1, y - height + 1, 3, height, Furniture::tree);
   int offsetTx = (chance(50) ? 0 : 3 * texSize);

   for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
         setBlock(tree.pieces[i][j], "grass", offsetTx + j * texSize, i * texSize);
      }
   }

   for (int i = 2; i < height; ++i) {
      auto& piece = tree.pieces[i][1];
      setBlock(piece, "planks", 2 * texSize, 3 * texSize);

      if (i + 1 == height) {
         bool rightFree = (map.isu(tree.posX + 2, tree.posY + i, Block::air) and isBlockSoil(map, tree.posX + 2, tree.posY + i + 1) and chance(25));
         bool leftFree = (map.isu(tree.posX, tree.posY + i, Block::air) and isBlockSoil(map, tree.posX, tree.posY + i + 1) and chance(25));

         if (rightFree) {
            setBlock(tree.pieces[i][2], "planks", 4 * texSize, 4 * texSize);
            piece.tx = 3 * texSize;
            piece.ty = 4 * texSize;
         }
         if (leftFree) {
            setBlock(tree.pieces[i][0], "planks", 0 * texSize, 4 * texSize);
            piece.tx = 1 * texSize;
            piece.ty = 4 * texSize;
         }
         if (rightFree and leftFree) {
            piece.tx = 5 * texSize;
            piece.ty = 4 * texSize;
         } else if (not rightFree and not leftFree) {
            piece.ty = 4 * texSize;
         }
      } else {
         bool rightFree = (map.is(tree.posX + 2, tree.posY + i, Block::air) and chance(15));
         bool leftFree = (map.is(tree.posX, tree.posY + i, Block::air) and chance(15));

         if (rightFree) {
            setBlock(tree.pieces[i][2], "planks", 4 * texSize, 3 * texSize);
            piece.tx = 3 * texSize;
            piece.tx = 3 * texSize;
         }
         if (leftFree) {
            setBlock(tree.pieces[i][0], "planks", 0 * texSize, 3 * texSize);
            piece.tx = 1 * texSize;
            piece.ty = 3 * texSize;
         }
         if (rightFree and leftFree) {
            piece.tx = 5 * texSize;
            piece.ty = 3 * texSize;
         }
      }
   }
   map.addFurniture(tree);
}
