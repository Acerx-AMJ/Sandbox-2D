#pragma once
#include "types.hpp"
#include <raylib.h>
#include <string>
#include <vector>

// Constants

constexpr inline float previewAlpha = 0.75f;

// Furniture data

enum class FurnitureType: unsigned char {
   none,
   tree,
   sapling,
   table,
   chair,
   door
};

struct FurnitureData {
   FurnitureType type = FurnitureType::none;
   Texture texture;
   int textureSize = 8;
   int treeSizeMin = 0;
   int treeSizeMax = 0;
   int treeRootChance = 0;
   int treeBranchChance = 0;
   bool treeIsCactus = false;
   int treeCactusFlowerChance = 0;
   furnitureid_t saplingGrowsInto = 0;
   float saplingGrowSpeedMin = 0.0f;
   float saplingGrowSpeedMax = 0.0f;
   Vector2 furnitureSize = {0, 0};
   bool shouldFacePlayer = false;
};

struct FurniturePiece {
   unsigned short tx = 0;
   unsigned short ty = 0;
   bool nil = false;
   bool walkable = false;
};

// Furniture getter functions

bool isFurnitureTypeValid(const std::string &name);
FurnitureType getFurnitureTypeFromString(const std::string &name);

bool isSaplingSoil(blockid_t id);
bool isTreeSoil(blockid_t id);
bool isSaplingSoilCompatible(blockid_t soilId, furnitureid_t saplingId);
bool isTreeSoilCompatible(blockid_t soilId, furnitureid_t treeId);
std::vector<furnitureid_t> &getSaplingsFromSoil(blockid_t id);
std::vector<furnitureid_t> &getTreesFromSoil(blockid_t id);

bool isValidFurnitureName(const std::string &name);
std::string getFurnitureNameFromId(furnitureid_t id);
furnitureid_t getFurnitureIdFromName(const std::string &name);
FurnitureType getFurnitureType(furnitureid_t id);
FurnitureData &getFurnitureData(furnitureid_t id);
size_t getFurnitureCount();

void reserveFurnitureContainers(size_t estimate);
furnitureid_t pushFurniture(FurnitureData data, const std::string &name, const std::vector<blockid_t> &saplingSoils, const std::vector<blockid_t> &treeSoils);

// furniture

struct Furniture {
   void init(furnitureid_t id, int x, int y, int width, int height);

   bool isSolidUnderneath(const struct Map &map, FurnitureData &data, bool previewing) const;
   bool isSuitableForPlant(const struct Map &map, FurnitureData &data, bool previewing) const;

   bool setSimpleFurniture(const struct Map &map, FurnitureData &data, bool playerFacingLeft, bool walkable, bool previewing);
   bool setPlant(const struct Map &map, FurnitureData &data, bool previewing);

   // Update functions

   void destroy(struct Map &map);
   void update(struct Map &map, struct Player &player, const Vector2 &mousePos, float dt);
   bool isValid(FurnitureData &data, const struct Map &map) const;

   // Render functions

   void preview(const struct Map &map) const;
   void render(const Rectangle &cameraBounds) const;

   // Members

   furnitureid_t id = 0;
   std::vector<FurniturePiece> pieces;

   int x = 0;
   int y = 0;
   int width = 0;
   int height = 0;

   int ivalue1 = 0;
   int ivalue2 = 0;
   float fvalue1 = 0;
   float fvalue2 = 0;
   bool deleted = false;
};

// Furniture generation functions

Furniture getFurniture(int x, int y, const struct Map &map, furnitureid_t id, bool playerFacingLeft, bool previewing = false);
void generateFurniture(int x, int y, struct Map &map, furnitureid_t id, bool playerFacingLeft);
