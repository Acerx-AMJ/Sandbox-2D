#include "mngr/data.hpp"
#include "SRU/assets.hpp"
#include "objs/map.hpp"
#include <SRU/file.hpp>
#include <cstdio>

// data functions

void loadData() {
   printf("Loading block data from 'assets/blocks_list.txt'...\n");
   loadBlockData();
   printf("Loading liquid data from 'assets/liquid_list.txt'...\n");
   loadLiquidData();
   printf("Loading furniture data from 'assets/furniture_list.txt'...\n");
   loadFurnitureData();
   printf("Loading done!\n");
}

void loadBlockData() {
   std::vector<Header> headers = getHeadersFromConfig("assets/blocks_list.txt", "#", "[", "]", '=');
   reserveBlockContainers(headers.size());

   for (Header &header: headers) {
      Texture texture {0};
      BlockType types {};
      bool noTexture = false;

      for (auto &[field, value]: header.lines) {
         if (field == "texture") {
            if (value.empty()) {
               texture.id = 0;
               noTexture = true;
            }
            else {
               texture = getTexture(value);
            }
         }
         else if (field == "attributes") {
            std::vector<std::string> attributes = getArrayValue(value);
            for (std::string &attribute: attributes) {
               if (!isBlockTypeValid(attribute)) {
                  printf("loadBlockData: Invalid block attribute '%s'.\n", attribute.c_str());
                  continue;
               }
               types = types | getBlockTypeFromString(attribute);
            }
         }
         else {
            printf("loadBlockData: Invalid field '%s'.\n", field.c_str());
         }
      }

      if (!noTexture && texture.id == 0) {
         texture = getTexture(header.name);
      }
      pushBlock(header.name, types, texture);
   }
}

void loadLiquidData() {
   std::vector<Header> headers = getHeadersFromConfig("assets/liquid_list.txt", "#", "[", "]", '=');
   reserveLiquidContainers(headers.size());

   // pre-pass. no avoiding because of the conversion table
   for (Header &header: headers) {
      pushLiquid(header.name);
   }

   for (Header &header: headers) {
      LiquidData data;
      bool noTexture = false;

      for (auto &[field, value]: header.lines) {
         if (field == "texture") {
            if (value.empty()) {
               data.texture.id = 0;
               noTexture = true;
            }
            else {
               data.texture = getTexture(value);
            }
         }
         else if (field == "update_speed") {
            data.updateSpeed = getFloatValue(value);
         }
         else if (field == "move_speed_multiplier") {
            data.moveSpeedMultiplier = getFloatValue(value);
         }
         else if (field == "natural_light") {
            data.naturalLight = getBoolValue(value);
         }
         else if (field == "glow") {
            data.glow = getBoolValue(value);
         }
         else if (field == "conversion") {
            std::vector<Line> dictionary = getDictionaryValue(value, '=');
            for (auto &[field, value]: dictionary) {
               if (!isBlockNameValid(value)) {
                  printf("loadLiquidData: Block '%s' does not exist.\n", value.c_str());
                  continue;
               }

               if (!isLiquidNameValid(field)) {
                  printf("loadLiquidData: Liquid '%s' does not exist.\n", field.c_str());
                  continue;
               }
               data.conversionTable[getLiquidIdFromName(field)] = getBlockIdFromName(value);
            }
         }
         else {
            printf("loadLiquidData: Invalid field '%s'.\n", field.c_str());
         }
      }

      if (!noTexture && data.texture.id == 0) {
         data.texture = getTexture(header.name);
      }
      setLiquid(header.name, data);
   }
}

void loadFurnitureData() {
   std::vector<Header> headers = getHeadersFromConfig("assets/furniture_list.txt", "#", "[", "]", '=');
   reserveFurnitureContainers(headers.size());

   for (Header &header: headers) {
      FurnitureData data;
      bool noTexture = false;
      std::vector<blockid_t> saplingSoils, treeSoils;

      for (auto &[field, value]: header.lines) {
         if (field == "texture") {
            if (value.empty()) {
               data.texture.id = 0;
               noTexture = true;
            }
            else {
               data.texture = getTexture(value);
            }
         }
         else if (field == "type") {
            if (!isFurnitureTypeValid(value)) {
               printf("loadFurnitureData: Invalid type '%s'.\n", value.c_str());
               continue;
            }
            data.type = getFurnitureTypeFromString(value);
         }
         else if (field == "texture_size") {
            data.textureSize = getIntValue(value);
         }
         else if (field == "tree_size_min") {
            data.treeSizeMin = getIntValue(value);
         }
         else if (field == "tree_size_max") {
            data.treeSizeMax = getIntValue(value);
         }
         else if (field == "tree_root_chance") {
            data.treeRootChance = getIntValue(value);
         }
         else if (field == "tree_branch_chance") {
            data.treeBranchChance = getIntValue(value);
         }
         else if (field == "tree_is_cactus") {
            data.treeIsCactus = getBoolValue(value);
         }
         else if (field == "tree_cactus_flower_chance") {
            data.treeCactusFlowerChance = getIntValue(value);
         }
         else if (field == "sapling_grows_into") {
            if (!isValidFurnitureName(value)) {
               printf("loadFurnitureData: No such furniture type '%s'. %s must be defined before this line.\n", value.c_str(), value.c_str());
               continue;
            }
            data.saplingGrowsInto = getFurnitureIdFromName(value);
         }
         else if (field == "sapling_grow_speed_min") {
            data.saplingGrowSpeedMin = getFloatValue(value);
         }
         else if (field == "sapling_grow_speed_max") {
            data.saplingGrowSpeedMax = getFloatValue(value);
         }
         else if (field == "size") {
            data.furnitureSize = getV2Value(value);
         }
         else if (field == "face_player") {
            data.shouldFacePlayer = getBoolValue(value);
         }
         else if (field == "sapling_soil") {
            std::vector<std::string> array = getArrayValue(value);
            for (std::string &soil: array) {
               if (!isBlockNameValid(soil)) {
                  printf("loadFurnitureData: Block '%s' does not exist.\n", soil.c_str());
                  continue;
               }
               saplingSoils.push_back(getBlockIdFromName(soil));
            }
         }
         else if (field == "tree_soil") {
            std::vector<std::string> array = getArrayValue(value);
            for (std::string &soil: array) {
               if (!isBlockNameValid(soil)) {
                  printf("loadFurnitureData: Block '%s' does not exist.\n", soil.c_str());
                  continue;
               }
               treeSoils.push_back(getBlockIdFromName(soil));
            }
         }
         else {
            printf("loadFurnitureData: Invalid field '%s'.\n", field.c_str());
         }
      }

      if (!noTexture && data.texture.id == 0) {
         data.texture = getTexture(header.name);
      }
      pushFurniture(data, header.name, saplingSoils, treeSoils);
   }
}
