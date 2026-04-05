#ifndef OBJS_CONSOLE_HPP
#define OBJS_CONSOLE_HPP

#include "ui/input.hpp"
#include <unordered_map>
#include <variant>
#include <vector>

// Console

enum class ConsoleColor: char {white, gray, yellow, red, green, blue, orange, purple, pink, count};
using VArgs = std::vector<std::string>;
using Variable = std::variant<bool*, int*, float*>;

struct Console {
   void init(struct Map & map, struct Player &player, struct Inventory &inventory);
   void update(float dt, struct Map &map, struct Player &player, struct Inventory &inventory);
   void updateResponsiveness();
   
   void render();

   void output(const std::string &string, ConsoleColor color = ConsoleColor::white);
   void lex(struct Map &map, struct Player &player, struct Inventory &inventory);
   bool handleCommand(VArgs &args, struct Map &map, struct Player &player, struct Inventory &inventory);

   // Members

   std::unordered_map<std::string, Variable> vars;
   std::vector<std::string> text, history;
   std::vector<ConsoleColor> textColors;

   Input input;
   int scrollback = 0;
   int historyIndex = 0;
};

#endif
