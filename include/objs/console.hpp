#pragma once
#include "ui/input.hpp"
#include <unordered_map>
#include <variant>
#include <vector>

using VArgs = std::vector<std::string>;
using Variable = std::variant<bool*, int*, float*>;

struct Console {
   void init(struct Map & map, struct Player &player, struct Inventory &inventory);
   void update(float dt, struct Map &map, struct Player &player, struct Inventory &inventory);
   void updateResponsiveness();
   
   void render();

   void output(const std::string &string, Color color = WHITE);
   void lex(struct Map &map, struct Player &player, struct Inventory &inventory);
   bool handleCommand(VArgs &args, struct Map &map, struct Player &player, struct Inventory &inventory);

   // Members

   std::unordered_map<std::string, Variable> vars;
   std::vector<std::string> text, history;
   std::vector<Color> textColors;

   Input input;
   int scrollback = 0;
   int historyIndex = 0;
};
