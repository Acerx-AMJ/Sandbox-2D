#include "objs/furniture.hpp"
#include "SRU/random.hpp"
#include "objs/map.hpp"
#include "objs/player.hpp"
#include "SRU/util.hpp"
#include <raymath.h>
#include <unordered_map>

// furniture info

constexpr int treeWidth = 3;
constexpr int furnitureInteractionRange = 8.0f;

static const std::unordered_map<std::string, FurnitureType> furnitureTypeStrings {{
   {"none", FurnitureType::none}, {"tree", FurnitureType::tree}, {"sapling", FurnitureType::sapling}, {"table", FurnitureType::table},
   {"chair", FurnitureType::chair}, {"door", FurnitureType::door},
}};

static size_t furnitureCount = 1; // 0 - nil
static std::vector<std::string> furnitureNames {""};
static std::vector<FurnitureData> furnitureData {{}};
static std::unordered_map<std::string, furnitureid_t> furnitureIds;
static std::unordered_map<blockid_t, std::vector<furnitureid_t>> saplingSoils, treeSoils;

// Furniture getter functions

bool isFurnitureTypeValid(const std::string &name) {
   return furnitureTypeStrings.find(name) != furnitureTypeStrings.end();
}

FurnitureType getFurnitureTypeFromString(const std::string &name) {
   if (auto it = furnitureTypeStrings.find(name); it != furnitureTypeStrings.end()) {
      return it->second;
   }
   return FurnitureType::none;
}

bool isSaplingSoil(blockid_t id) {
   return saplingSoils.find(id) != saplingSoils.end();
}

bool isTreeSoil(blockid_t id) {
   return treeSoils.find(id) != treeSoils.end();
}

bool isSaplingSoilCompatible(blockid_t soilId, furnitureid_t saplingId) {
   if (auto it = saplingSoils.find(soilId); it != saplingSoils.end()) {
      for (furnitureid_t sapling: it->second) {
         if (sapling == saplingId) {
            return true;
         }
      }
   }
   return false;
}

bool isTreeSoilCompatible(blockid_t soilId, furnitureid_t treeId) {
   if (auto it = treeSoils.find(soilId); it != treeSoils.end()) {
      for (furnitureid_t tree: it->second) {
         if (tree == treeId) {
            return true;
         }
      }
   }
   return false;
}

std::vector<furnitureid_t> &getSaplingsFromSoil(blockid_t id) {
   return saplingSoils.at(id);;
}

std::vector<furnitureid_t> &getTreesFromSoil(blockid_t id) {
   return treeSoils.at(id);;
}

furnitureid_t getFurnitureIdFromName(const std::string &name) {
   return furnitureIds.at(name);
}

bool isFurnitureIdValid(furnitureid_t id) {
   return id >= 0 && id < furnitureCount;
}

bool isFurnitureNameValid(const std::string &name) {
   return furnitureIds.find(name) != furnitureIds.end();
}

std::string getFurnitureNameFromId(furnitureid_t id) {
   return furnitureNames[id];
}

FurnitureType getFurnitureType(furnitureid_t id) {
   return furnitureData[id].type;
}

FurnitureData &getFurnitureData(furnitureid_t id) {
   return furnitureData[id];
}

size_t getFurnitureCount() {
   return furnitureCount;
}

void reserveFurnitureContainers(size_t estimate) {
   furnitureNames.reserve(estimate + 1);
   furnitureData.reserve(estimate + 1);
   furnitureIds.reserve(estimate + 1);
}

void pushFurniture(const std::string &name) {
   furnitureData.push_back({});
   furnitureNames.push_back(name);
   furnitureIds[name] = furnitureCount;
   furnitureCount += 1;
}

void setFurniture(const std::string &name, FurnitureData data, const std::vector<blockid_t> &saplingSoils, const std::vector<blockid_t> &treeSoils) {
   furnitureid_t id = furnitureIds[name];

   furnitureData[id] = data;
   for (blockid_t soil: saplingSoils) {
      ::saplingSoils[soil].push_back(id);
   }
   for (blockid_t soil: treeSoils) {
      ::treeSoils[soil].push_back(id);
   }
}

// Constructors

void Furniture::init(furnitureid_t id, int x, int y, int width, int height) {
   this->id = id;
   this->x = x;
   this->y = y;
   this->width = width;
   this->height = height;
   pieces = std::vector<FurniturePiece>(width * height, FurniturePiece{});
}

bool Furniture::isSolidUnderneath(const Map &map, FurnitureData &data, bool previewing) const {
   if (previewing) {
      return true;
   }
   for (int dx = x; dx < x + width; ++dx) {
      if (!map.is(dx, y + height, BlockType::solid)) {
         return false;
      }
   }
   return true;
}

bool Furniture::isSuitableForPlant(const Map &map, FurnitureData &data, bool previewing) const {
   if (previewing) {
      return true;
   }
   // make sure there's no liquids in the sapling and no sand on top of it. as well as is it on soil
   for (int dy = 0; dy < height; ++dy) {
      for (int dx = 0; dx < width; ++dx) {
         if (map.isLiquid(x + dx, y + dy) || (dy == 0 && map.is(x + dx, y - 1, BlockType::sand)) || (dy + 1 == height && !isSaplingSoilCompatible(map.getBlock(x + dx, y + height).id, id))) {
            return false;
         }
      }
   }
   return true;
}

bool Furniture::setSimpleFurniture(const Map &map, FurnitureData &data, bool playerFacingLeft, bool walkable, bool previewing) {
   int offset = (data.shouldFacePlayer && !playerFacingLeft ? data.textureSize * width : 0);

   for (int dy = 0; dy < height; ++dy) {
      for (int dx = 0; dx < width; ++dx) {
         if (!previewing && !map.isNotSolid(x + dx, y + dy)) {
            return false;
         }
         int i = dy * width + dx;
         pieces[i].tx = data.textureSize * dx + offset;
         pieces[i].ty = data.textureSize * dy;
         pieces[i].walkable = (dy == 0 && walkable);
      }
   }
   return true;
}

bool Furniture::setPlant(const Map &map, FurnitureData &data, bool previewing) {
   int textureWidth = data.textureSize * width;
   int offset = randomInt(0, data.texture.width / textureWidth - 1) * textureWidth;

   for (int dy = 0; dy < height; ++dy) {
      for (int dx = 0; dx < width; ++dx) {
         if (!previewing && !map.isEmpty(x + dx, y + dy)) {
            return false;
         }
         int i = dy * width + dx;
         pieces[i].tx = data.textureSize * dx + offset;
         pieces[i].ty = data.textureSize * dy;
      }
   }
   return true;
}

// furniture update functions

void Furniture::destroy(Map &map) {
   map.removeFurniture(*this);
}

void Furniture::update(Map &map, Player &player, const Vector2 &mousePos, float dt) {
   FurnitureData &data = furnitureData[id];
   if (!isValid(data, map)) {
      map.removeFurniture(*this);
      return;
   }
   
   switch (data.type) {
   case FurnitureType::sapling: {
      fvalue2 += dt;
      if (fvalue2 >= fvalue1) {
         map.removeFurniture(*this);
         generateFurniture(x + (width - 1) / 2, y + (height - 1), map, data.saplingGrowsInto, false);
      }
   } break;
   case FurnitureType::door: {
      Rectangle doorRect = R4(x, y, width, height);
      bool previousValue = ivalue1;
      bool previousValue2 = ivalue2;

      ivalue2 = CheckCollisionRecs(doorRect, player.getBounds());
      if (previousValue2 && !ivalue2 && (!ivalue1 || !ivalue2)) {
         ivalue1 = false; // Close the door
      } else if (!previousValue2 && ivalue2 && (!ivalue1 || !ivalue2)) {
         ivalue1 = true; // Open the door
      }

      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(mousePos, doorRect) && Vector2Distance({(float)x, (float)y}, player.getCenter()) < furnitureInteractionRange) {
         player.placedBlock = true;
         ivalue1 = !ivalue1;
         ivalue2 = false;
      }

      for (int i = 0; ivalue1 != previousValue && i < height; ++i) {
         pieces[i * width].ty = (ivalue1 * height) * data.textureSize;
      }
   } break;
   default: break;
   }
}

bool Furniture::isValid(FurnitureData &data, const Map &map) const {
   switch (data.type) {
   case FurnitureType::tree:
      return isTreeSoilCompatible(map.getBlock(x + treeWidth / 2, y + height).id, id);
   case FurnitureType::sapling:
      return isSuitableForPlant(map, data, false);
   case FurnitureType::table: case FurnitureType::chair: case FurnitureType::door:
      return isSolidUnderneath(map, data, false);
   default:
      return false;
   }
}

// Render furniture

void Furniture::preview(const Map &map) const {
   FurnitureData &data = furnitureData[id];
   bool valid = isValid(data, map);

   for (int dy = y; dy - y < height; ++dy) {
      for (int dx = x; dx - x < width; ++dx) {
         const FurniturePiece &piece = pieces[(dy - y) * width + (dx - x)];
         if (piece.nil) {
            continue;
         }
         Color color = Fade((map.isNotSolid(dx, dy) && valid ? WHITE : RED), furniturePreviewAlpha);
         DrawTexturePro(data.texture, R4(piece.tx, piece.ty, data.textureSize, data.textureSize), R4(dx, dy, 1, 1), {0, 0}, 0, color);
      }
   }
}

void Furniture::render(const Rectangle &cameraBounds) const {
   FurnitureData &data = furnitureData[id];
   for (int dy = y; dy <= cameraBounds.height && dy - y < height; ++dy) {
      for (int dx = x; dx <= cameraBounds.width && dx - x < width; ++dx) {
         const FurniturePiece &piece = pieces[(dy - y) * width + (dx - x)];
         if (dy < cameraBounds.y || dx < cameraBounds.x || piece.nil) {
            continue;
         }
         Color color = (data.type == FurnitureType::door && ivalue1 ? wallTint : WHITE);
         DrawTexturePro(data.texture, R4(piece.tx, piece.ty, data.textureSize, data.textureSize), R4(dx, dy, 1, 1), {0, 0}, 0, color);
      }
   }
}

// Furniture generation functions

Furniture getFurniture(int x, int y, const Map &map, furnitureid_t id, bool playerFacingLeft, bool previewing) {
   FurnitureData data = furnitureData[id];

   switch (data.type) {
   case FurnitureType::tree: {
      // trees are not placed by top-left but from center-bottom.
      int treeHeight = randomInt(data.treeSizeMin, data.treeSizeMax);
      for (int dy = 0; dy < treeHeight; ++dy) {
         if (!map.isNotSolid(x, y - dy)) {
            treeHeight = dy;
            break;
         }
      }

      if (!previewing && (treeHeight < data.treeSizeMin || !isTreeSoilCompatible(map.getBlock(x, y + 1).id, id))) {
         return {};
      }

      bool isPalm = (data.treeRootChance == 0 && data.treeBranchChance == 0 && !data.treeIsCactus);
      int middle = treeWidth / 2;
      int topHeight = 0;
   
      Furniture tree;
      tree.init(id, x - middle, y - treeHeight + 1, treeWidth, treeHeight);

      // place the tree top
      if (!data.treeIsCactus) {
         int topOffset = chance(50) * treeWidth * data.textureSize;
         topHeight = (isPalm ? 3 : 2);

         for (int dy = 0; dy < topHeight; ++dy) {
            for (int dx = 0; dx < treeWidth; ++dx) {
               int i = dy * treeWidth + dx;
               tree.pieces[i].tx = dx * data.textureSize + topOffset;
               tree.pieces[i].ty = dy * data.textureSize;
            }
         }
      }
      // initial cactus stub setup
      else {
         for (int dy = 0; dy < treeHeight; ++dy) {
            int middleI = dy * treeWidth + middle;
            int worldY = y - treeHeight + 1 + dy;
            tree.pieces[middleI-1].nil = (dy + 1 == treeHeight || dy == 0 || !map.isNotSolid(x - 1, worldY) || chance(100 - data.treeBranchChance));
            tree.pieces[middleI+1].nil = (dy + 1 == treeHeight || dy == 0 || !map.isNotSolid(x + 1, worldY) || chance(100 - data.treeBranchChance));
         }
      }

      // place the trunk, branches and roots
      int trunkHeight = treeHeight - topHeight;
      for (int dy = 0; dy < trunkHeight; ++dy) {
         int middleI = (dy + topHeight) * treeWidth + middle;
         int worldY = y - (trunkHeight - 1 - dy);

         if (isPalm) {
            int topOffset = (dy + 1 == trunkHeight ? 4 : 3);
            tree.pieces[middleI].tx = randomInt(0, 2) * data.textureSize;
            tree.pieces[middleI].ty = topOffset * data.textureSize;

            tree.pieces[middleI-1].nil = true;
            tree.pieces[middleI+1].nil = true;
         }
         else if (data.treeIsCactus) {
            bool rightStub = (!tree.pieces[middleI+1].nil && tree.pieces[middleI + treeWidth + 1].nil);
            bool leftStub = (!tree.pieces[middleI-1].nil && tree.pieces[middleI + treeWidth - 1].nil);
            bool anyStub = (rightStub || leftStub);

            // a lot of clever bool logic incoming. it just works and saves long if chains. I don't recommend tinkering too much
            // with cactus or tree sprite layouts and just going with the flow here. another yucky trick is not checking nil
            // on stubs and applying tx and ty anyway since nil pieces don't check them.
            int topOffset = anyStub * (rightStub + leftStub * 2) + !anyStub * ((dy + 1 == trunkHeight) * 3 + (dy == 0) * chance(data.treeCactusFlowerChance));
            int leftOffset = !anyStub * (dy == 0 || dy + 1 == trunkHeight);
            tree.pieces[middleI].tx = leftOffset * data.textureSize;
            tree.pieces[middleI].ty = topOffset * data.textureSize;

            int offsetXLeft = 2 * data.textureSize;
            int topOffsetLeft = (dy == 0 || tree.pieces[middleI - treeWidth - 1].nil) + (dy + 1 == trunkHeight || tree.pieces[middleI + treeWidth - 1].nil) * 2;
            tree.pieces[middleI-1].tx = offsetXLeft;
            tree.pieces[middleI-1].ty = topOffsetLeft * data.textureSize;

            int offsetXRight = 3 * data.textureSize;
            int topOffsetRight = (dy == 0 || tree.pieces[middleI - treeWidth + 1].nil) + (dy + 1 == trunkHeight || tree.pieces[middleI + treeWidth + 1].nil) * 2;
            tree.pieces[middleI+1].tx = offsetXRight;
            tree.pieces[middleI+1].ty = topOffsetRight * data.textureSize;
         }
         else {
            // some more clever bool logic here.
            bool isRoot = dy + 1 == trunkHeight;
            int percent = (isRoot ? data.treeRootChance : data.treeBranchChance);
            bool leftFree = map.isNotSolid(x - 1, worldY) && chance(percent) && (!isRoot || (isRoot && map.isSoil(x - 1, worldY + 1)));
            bool rightFree = map.isNotSolid(x + 1, worldY) && chance(percent) && (!isRoot || (isRoot && map.isSoil(x + 1, worldY + 1)));

            int topOffsetMiddle = (isRoot ? 4 : 3);
            int leftOffsetMiddle = (rightFree) * 3 + (leftFree) + (rightFree && leftFree) + (!leftFree && !rightFree) * 2;
            tree.pieces[middleI].tx = leftOffsetMiddle * data.textureSize;
            tree.pieces[middleI].ty = topOffsetMiddle * data.textureSize;

            int topOffsetBranches = (isRoot ? 4 : 2) * data.textureSize;
            int leftOffsetLeftBranch = (!isRoot) * randomInt(0, 2);
            int leftOffsetRightBranch = (isRoot ? 4 : randomInt(3, 5));
            tree.pieces[middleI-1].tx = leftOffsetLeftBranch * data.textureSize;
            tree.pieces[middleI-1].ty = topOffsetBranches;
            tree.pieces[middleI-1].nil = !leftFree;

            tree.pieces[middleI+1].tx = leftOffsetRightBranch * data.textureSize;
            tree.pieces[middleI+1].ty = topOffsetBranches;
            tree.pieces[middleI+1].nil = !rightFree;
         }
      }
      return tree;
   } break;
   case FurnitureType::sapling: {
      Furniture sapling;
      sapling.init(id, x, y, data.furnitureSize.x, data.furnitureSize.y);
      if (sapling.isSuitableForPlant(map, data, previewing) && sapling.setPlant(map, data, previewing)) {
         sapling.fvalue1 = randomFloat(data.saplingGrowSpeedMin, data.saplingGrowSpeedMax);
         return sapling;
      }
   } break;
   case FurnitureType::table: {
      Furniture table;
      table.init(id, x, y, data.furnitureSize.x, data.furnitureSize.y);
      if (table.isSolidUnderneath(map, data, previewing) && table.setSimpleFurniture(map, data, playerFacingLeft, true, previewing)) {
         return table;
      }
   } break;
   case FurnitureType::chair: {
      Furniture chair;
      chair.init(id, x, y, data.furnitureSize.x, data.furnitureSize.y);
      if (chair.isSolidUnderneath(map, data, previewing) && chair.setSimpleFurniture(map, data, playerFacingLeft, false, previewing)) {
         return chair;
      }
   } break;
   case FurnitureType::door: {
      Furniture door;
      door.init(id, x, y, data.furnitureSize.x, data.furnitureSize.y);
      if (door.isSolidUnderneath(map, data, previewing) && door.setSimpleFurniture(map, data, playerFacingLeft, false, previewing)) {
         return door;
      }
   } break;
   default: break;
   }
   return {};
}

void generateFurniture(int x, int y, Map &map, furnitureid_t type, bool playerFacingleft) {
   Furniture furniture = getFurniture(x, y, map, type, playerFacingleft);
   if (furniture.id != 0) {
      map.addFurniture(furniture);
   }
}
