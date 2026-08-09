#include "mngr/data.hpp"
#include "SRU/assets.hpp"
#include "SRU/text.hpp"
#include "objs/map.hpp"
#include <SRU/file.hpp>
#include <cstdio>
#include <sstream>

template<typename T>
void getFieldAsSimpleValue(std::stringstream &stream, const std::string &value, const std::string &line, T &target) {
   stream.clear();
   stream.str(value);
   stream >> std::boolalpha >> target;
   if (stream.rdbuf()->in_avail() != 0) {
      printf("WARNING: Malformed line '%s'. Expected number/boolean.\n", line.c_str());
   }
}

void loadData() {
   printf("Loading block data from 'assets/blocks_list.txt'...\n");
   loadBlockData();
   printf("Loading done!\n");
}

void loadBlockData() {
   std::vector<std::string> lines = getLinesFromFileIgnoringComments("assets/blocks_list.txt", "#");
   std::stringstream stream;

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
         data.texture = getTexture(value);
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
   }

   if (init) {
      data.push();
   }
}
