#ifndef OBJS_CONSOLE_HPP
#define OBJS_CONSOLE_HPP

#include "ui/input.hpp"
#include <vector>

// Commands

#define VArgs std::vector<std::string>
#define ArgsList struct Console &console, const std::string &original, const VArgs &args, struct Map &map, struct Player &player, struct Inventory &inventory

bool c_help(ArgsList);
bool c_tp(ArgsList);
bool c_sp(ArgsList);
bool c_crds(ArgsList);
bool c_clear(ArgsList);
bool c_exit(ArgsList);
bool c_hp(ArgsList);
bool c_maxhp(ArgsList);
bool c_br(ArgsList);
bool c_kill(ArgsList);

// Console

enum class ConsoleColor: char {white, gray, yellow, red, green, blue, orange, purple, pink, count};

struct Console {
   void init();   
   void update(struct Map &map, struct Player &player, struct Inventory &inventory);
   void render();

   void output(const std::string &string, ConsoleColor color = ConsoleColor::white);
   void lex(struct Map &map, struct Player &player, struct Inventory &inventory);
   bool handleCommand(const VArgs &args, struct Map &map, struct Player &player, struct Inventory &inventory);

   // Members

   std::vector<std::string> text;
   std::vector<ConsoleColor> textColors;

   Input input;
   int scrollback = 0;
};

#endif
