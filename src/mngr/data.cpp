#include "mngr/data.hpp"
#include "SRU/assets.hpp"
#include "SRU/text.hpp"
#include "objs/map.hpp"
#include <SRU/file.hpp>
#include <cstdio>

// helper functions

void getIntValue(const std::string &value, const std::string &line, int &target) {
   // disgusting
   try {
      target = std::stoi(value);
   } catch (...) {
      printf("WARNING: Malformed line '%s'. Expected integer.\n", line.c_str());
   }
}

void getFloatValue(const std::string &value, const std::string &line, float &target) {
   // even more disgusting
   try {
      target = std::stof(value);
   } catch (...) {
      printf("WARNING: Malformed line '%s'. Expected real number.\n", line.c_str());
   }
}

void getBoolValue(const std::string &value, const std::string &line, bool &target) {
   if (value == "true") {
      target = true;
   }
   else if (value == "false") {
      target = false;
   }
   else {
      printf("WARNING: Malformed line '%s'. Expected true/false.\n", line.c_str());
   }
}

void getV2Value(const std::string &value, const std::string &line, Vector2 &target) {
   std::vector<std::string> values = split(value, ',');
   if (values.size() != 2) {
      printf("WARNING: Malformed line '%s'. Expected Vector2 (NUMBER,NUMBER).\n", line.c_str());
      return;
   }
   getFloatValue(values[0], line, target.x);
   getFloatValue(values[1], line, target.y);
}

// data functions

void loadData() {
   printf("Loading block data from 'assets/blocks_list.txt'...\n");
   loadBlockData();
   printf("Loading furniture data from 'assets/furniture_list.txt'...\n");
   loadFurnitureData();
   printf("Loading done!\n");
}

void loadBlockData() {
   std::vector<std::string> lines = getLinesFromFileIgnoringComments("assets/blocks_list.txt", "#");
   reserveBlockContainers(lines.size() / 2);

   struct BlockData {
      std::string name;
      Texture texture {0};
      BlockType types;
      bool noTexture = false;

      void push() {
         if (!noTexture && texture.id == 0) {
            texture = getTexture(name);
         }
         pushBlock(name, types, texture);
         *this = {};
      }
   };
   BlockData data;
   bool init = false;

   for (const std::string &line: lines) {
      // getLinesFromFileIgnoringComments skips empty lines so this is fine
      if (line.front() == '[' && line.back() == ']') {
         if (init) {
            data.push();
         }
         data.name = line.substr(1, line.size() - 2);
         init = true;
         continue;
      }

      size_t equals = line.find('=');
      if (equals == std::string::npos) {
         printf("WARNING: Malformed line: '%s'. Expected '=' character.\n", line.c_str());
         continue;
      }

      std::string field = line.substr(0, equals);
      std::string value = line.substr(equals + 1);
      trimRightInPlace(field);
      trimLeftInPlace(value);

      if (field == "texture") {
         if (value.empty()) {
            data.noTexture = true;
            data.texture.id = 0;
         } else {
            data.texture = getTexture(value);
         }
      }
      else if (field == "attributes") {
         std::vector<std::string> attributes = split(value, ',');
         for (std::string &attribute: attributes) {
            std::string trimmed = trim(attribute);
            if (!isBlockTypeValid(trimmed)) {
               printf("WARNING: Malformed line: '%s'. Invalid attribute '%s'.\n", line.c_str(), trimmed.c_str());
               continue;
            }
            data.types = data.types | getBlockTypeFromString(trimmed);
         }
      }
      else {
         printf("WARNING: Malformed line: '%s'. Invalid field '%s'.\n", line.c_str(), field.c_str());
      }
   }

   if (init) {
      data.push();
   }
}

void loadFurnitureData() {
   std::vector<std::string> lines = getLinesFromFileIgnoringComments("assets/furniture_list.txt", "#");
   reserveFurnitureContainers(lines.size() / 5);

   FurnitureData data;
   bool init = false;
   bool noTexture = false;
   std::string name;
   std::vector<blockid_t> saplingSoils, treeSoils;

   for (const std::string &line: lines) {
      // getLinesFromFileIgnoringComments skips empty lines so this is fine
      if (line.front() == '[' && line.back() == ']') {
         if (init) {
            if (!noTexture && data.texture.id == 0) {
               data.texture = getTexture(name);
            }
            pushFurniture(data, name, saplingSoils, treeSoils);
            data = {};
            noTexture = false;
            name.clear();
            saplingSoils.clear();
            treeSoils.clear();
         }
         name = line.substr(1, line.size() - 2);
         init = true;
         continue;
      }

      size_t equals = line.find('=');
      if (equals == std::string::npos) {
         printf("WARNING: Malformed line: '%s'. Expected '=' character.\n", line.c_str());
         continue;
      }

      std::string field = line.substr(0, equals);
      std::string value = line.substr(equals + 1);
      trimRightInPlace(field);
      trimLeftInPlace(value);

      if (field == "texture") {
         if (value.empty()) {
            noTexture = true;
            data.texture.id = 0;
         } else {
            data.texture = getTexture(value);
         }
      }
      else if (field == "type") {
         if (!isFurnitureTypeValid(value)) {
            printf("WARNING: Malformed line: '%s'. Invalid type '%s'.\n", line.c_str(), value.c_str());
            continue;
         }
         data.type = getFurnitureTypeFromString(value);
      }
      else if (field == "texture_size") {
         getIntValue(value, line, data.textureSize);
      }
      else if (field == "tree_size_min") {
         getIntValue(value, line, data.treeSizeMin);
      }
      else if (field == "tree_size_max") {
         getIntValue(value, line, data.treeSizeMax);
      }
      else if (field == "tree_root_chance") {
         getIntValue(value, line, data.treeRootChance);
      }
      else if (field == "tree_branch_chance") {
         getIntValue(value, line, data.treeBranchChance);
      }
      else if (field == "tree_is_cactus") {
         getBoolValue(value, line, data.treeIsCactus);
      }
      else if (field == "tree_cactus_flower_chance") {
         getIntValue(value, line, data.treeCactusFlowerChance);
      }
      else if (field == "sapling_grows_into") {
         if (!isValidFurnitureName(value)) {
            printf("WARNING: Malformed line: '%s'. No such furniture type '%s'. %s must be defined before this line.\n", line.c_str(), value.c_str(), value.c_str());
            continue;
         }
         data.saplingGrowsInto = getFurnitureIdFromName(value);
      }
      else if (field == "sapling_grow_speed_min") {
         getFloatValue(value, line, data.saplingGrowSpeedMin);
      }
      else if (field == "sapling_grow_speed_max") {
         getFloatValue(value, line, data.saplingGrowSpeedMax);
      }
      else if (field == "size") {
         getV2Value(value, line, data.furnitureSize);
      }
      else if (field == "face_player") {
         getBoolValue(value, line, data.shouldFacePlayer);
      }
      else if (field == "sapling_soil") {
         std::vector<std::string> soils = split(value, ',');
         for (std::string &soil: soils) {
            std::string trimmed = trim(soil);
            if (!isBlockNameValid(trimmed)) {
               printf("WARNING: Malformed line: '%s'. Block '%s' does not exist.\n", line.c_str(), trimmed.c_str());
               continue;
            }
            saplingSoils.push_back(getBlockIdFromName(trimmed));
         }
      }
      else if (field == "tree_soil") {
         std::vector<std::string> soils = split(value, ',');
         for (std::string &soil: soils) {
            std::string trimmed = trim(soil);
            if (!isBlockNameValid(trimmed)) {
               printf("WARNING: Malformed line: '%s'. Block '%s' does not exist.\n", line.c_str(), trimmed.c_str());
               continue;
            }
            treeSoils.push_back(getBlockIdFromName(trimmed));
         }
      }
      else {
         printf("WARNING: Malformed line: '%s'. Invalid field '%s'.\n", line.c_str(), field.c_str());
      }
   }

   if (init) {
      pushFurniture(data, name, saplingSoils, treeSoils);
   }
}
