#include "mngr/data.hpp"
#include "SRU/assets.hpp"
#include "SRU/text.hpp"
#include "objs/item.hpp"
#include "objs/map.hpp"
#include <SRU/file.hpp>
#include <cstdio>

// data functions

void loadData() {
   printf("Reading all config files...\n");
   std::vector<Header> blockHeaders = getHeadersFromConfig("assets/config/blocks.txt", "#", "[", "]", '=');
   std::vector<Header> liquidHeaders = getHeadersFromConfig("assets/config/liquids.txt", "#", "[", "]", '=');
   std::vector<Header> furnitureHeaders = getHeadersFromConfig("assets/config/furniture.txt", "#", "[", "]", '=');
   std::vector<Header> itemHeaders = getHeadersFromConfig("assets/config/items.txt", "#", "[", "]", '=');
   std::vector<Header> dropHeaders = getHeadersFromConfig("assets/config/drop_tables.txt", "#", "[", "]", '=');

   printf("Sizing all containers...\n");
   reserveBlockContainers(blockHeaders.size());
   reserveLiquidContainers(liquidHeaders.size());
   reserveFurnitureContainers(furnitureHeaders.size());
   reserveItemContainers(itemHeaders.size());
   reserveDropTableContainers(dropHeaders.size());

   printf("Running pre-pass for all files...\n");
   loadPrepass(blockHeaders, liquidHeaders, furnitureHeaders, itemHeaders, dropHeaders);

   printf("Loading block data from 'assets/config/blocks.txt'...\n");
   loadBlockData(blockHeaders);
   printf("Loading liquid data from 'assets/config/liquids.txt'...\n");
   loadLiquidData(liquidHeaders);
   printf("Loading furniture data from 'assets/config/furniture.txt'...\n");
   loadFurnitureData(furnitureHeaders);
   printf("Loading item data from 'assets/config/items.txt'...\n");
   loadItemData(itemHeaders);
   printf("Loading drop table data from 'assets/config/drop_tables.txt'...\n");
   loadDropTableData(dropHeaders);
   printf("Loading done!\n");
}

void loadPrepass(std::vector<Header> &blockHeaders, std::vector<Header> &liquidHeaders, std::vector<Header> &furnitureHeaders, std::vector<Header> &itemHeaders, std::vector<Header> &dropHeaders) {
   for (Header &header: blockHeaders) {
      pushBlock(header.name);
   }

   for (Header &header: liquidHeaders) {
      pushLiquid(header.name);
   }

   for (Header &header: furnitureHeaders) {
      pushFurniture(header.name);
   }

   for (Header &header: itemHeaders) {
      pushItem(header.name);
   }

   for (Header &header: dropHeaders) {
      pushDropTable(header.name);
   }
}

void loadBlockData(std::vector<Header> &headers) {
   for (Header &header: headers) {
      BlockData data;
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
         else if (field == "drop_table") {
            if (!isDropTableNameValid(value)) {
               printf("loadBlockData: Drop table '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.dropTable = getDropTableIdFromName(value);
         }
         else if (field == "break_speed") {
            data.breakSpeed = getFloatValue(value);
         }
         else if (field == "tool_power") {
            data.toolPower = getIntValue(value);
         }
         else if (field == "tool_power_required") {
            data.toolPowerRequired = getBoolValue(value);
         }
         else if (field == "tool_type") {
            if (!isToolTypeValid(value)) {
               printf("loadBlockData: Tool type '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.toolType = getToolTypeFromString(value);
         }
         else if (field == "drop_table_wall") {
            if (!isDropTableNameValid(value)) {
               printf("loadBlockData: Drop table '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.wallDropTable = getDropTableIdFromName(value);
         }
         else if (field == "break_speed_wall") {
            data.wallBreakSpeed = getFloatValue(value);
         }
         else if (field == "tool_power_wall") {
            data.wallToolPower = getIntValue(value);
         }
         else if (field == "tool_power_required_wall") {
            data.wallToolPowerRequired = getBoolValue(value);
         }
         else if (field == "tool_type_wall") {
            if (!isToolTypeValid(value)) {
               printf("loadBlockData: Tool type '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.wallToolType = getToolTypeFromString(value);
         }
         else if (field == "attributes") {
            std::vector<std::string> attributes = getArrayValue(value);
            for (std::string &attribute: attributes) {
               if (!isBlockTypeValid(attribute)) {
                  printf("loadBlockData: Invalid block attribute '%s'.\n", attribute.c_str());
                  continue;
               }
               data.attributes = data.attributes | getBlockTypeFromString(attribute);
            }
         }
         else {
            printf("loadBlockData: Invalid field '%s'.\n", field.c_str());
         }
      }

      if (!noTexture && data.texture.id == 0) {
         data.texture = getTexture(header.name);
      }
      setBlock(header.name, data);
   }
}

void loadLiquidData(std::vector<Header> &headers) {
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
            data.updateSpeed = getIntValue(value);
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
         else if (field == "damage_player") {
            data.damagePlayer = getBoolValue(value);
         }
         else if (field == "damage_min") {
            data.damageMin = getIntValue(value);
         }
         else if (field == "damage_max") {
            data.damageMax = getIntValue(value);
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

void loadFurnitureData(std::vector<Header> &headers) {
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
            if (!isFurnitureNameValid(value)) {
               printf("loadFurnitureData: No such furniture type '%s'.\n", value.c_str());
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
         else if (field == "drop_table") {
            if (!isDropTableNameValid(value)) {
               printf("loadFurnitureData: Drop table '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.dropTable = getDropTableIdFromName(value);
         }
         else if (field == "break_speed") {
            data.breakSpeed = getFloatValue(value);
         }
         else if (field == "tool_power") {
            data.toolPower = getIntValue(value);
         }
         else if (field == "tool_power_required") {
            data.toolPowerRequired = getBoolValue(value);
         }
         else if (field == "tool_type") {
            if (!isToolTypeValid(value)) {
               printf("loadFurnitureData: Tool type '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.toolType = getToolTypeFromString(value);
         }
         else {
            printf("loadFurnitureData: Invalid field '%s'.\n", field.c_str());
         }
      }

      if (!noTexture && data.texture.id == 0) {
         data.texture = getTexture(header.name);
      }
      setFurniture(header.name, data, saplingSoils, treeSoils);
   }
}

void loadItemData(std::vector<Header> &headers) {
   for (Header &header: headers) {
      ItemData data;
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
         else if (field == "block") {
            if (!isBlockNameValid(value)) {
               printf("loadItemData: Block '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.block = getBlockIdFromName(value);
         }
         else if (field == "wall") {
            if (!isBlockNameValid(value)) {
               printf("loadItemData: Wall '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.wall = getBlockIdFromName(value);
         }
         else if (field == "furniture") {
            if (!isFurnitureNameValid(value)) {
               printf("loadItemData: Furniture '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.furniture = getFurnitureIdFromName(value);
         }
         else if (field == "liquid") {
            if (!isLiquidNameValid(value)) {
               printf("loadItemData: Liquid '%s' does not exist.\n", value.c_str());
               continue;
            }
            data.liquid = getLiquidIdFromName(value);
         }
         else if (field == "stack_size") {
            data.stackSize = getIntValue(value);
         }
         else if (field == "action") {
            if (!isItemActionValid(value)) {
               printf("loadItemData: Invalid action '%s'.\n", value.c_str());
               continue;
            }
            data.action = getItemActionTypeFromString(value);
         }
         else {
            printf("loadItemData: Invalid field '%s'.\n", field.c_str());
         }
      }

      if (!noTexture && data.texture.id == 0) {
         data.texture = getTexture(header.name);
      }
      setItem(header.name, data);
   }
}

void loadDropTableData(std::vector<Header> &headers) {
   for (Header &header: headers) {
      DropTable data;

      for (auto &[field, value]: header.lines) {
         if (field == "table") {
            std::vector<Line> dictionary = getDictionaryValue(value, '=');
            data.drops.reserve(data.drops.size() + dictionary.size()); // in case someone defines table multiple times
            
            for (auto &[item, values]: dictionary) {
               std::vector<std::string> array = clean(split(values, ';'));
               if (array.size() > 3) {
                  printf("loadDropTableData: Too many values for drop table '%s=%s'.\n", item.c_str(), values.c_str());
                  continue;
               }
               if (!isItemNameValid(item)) {
                  printf("loadDropTableData: Item '%s' does not exist.\n", item.c_str());
                  continue;
               }
               itemid_t id = getItemIdFromName(item);
               float dropChance = (array.size() > 0 ? getFloatValue(array[0]) : 1.0f);
               int dropMin = (array.size() > 1 ? getIntValue(array[1]) : 1);
               int dropMax = (array.size() > 2 ? getIntValue(array[2]) : 1);
               data.drops.emplace_back(id, dropChance, dropMin, dropMax);
            }
         }
         else {
            printf("loadDropTableData: Invalid field '%s'.\n", field.c_str());
         }
      }
      setDropTable(header.name, data);
   }
}
