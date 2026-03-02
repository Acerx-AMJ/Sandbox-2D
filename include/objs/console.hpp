#ifndef OBJS_CONSOLE_HPP
#define OBJS_CONSOLE_HPP

#include "ui/input.hpp"
#include <vector>

struct Console {
   void init();
   void divideOutput(const std::string &string);
   
   void update(struct Map &map, struct Player &player, struct Inventory &inventory);
   void render();

   // Commands

   using Args = std::vector<std::string>;

   void handleCommand(struct Map &map, struct Player &player, struct Inventory &inventory);
   void help(const Args &args, Map &map, Player &player, Inventory &inventory);
   void tp(const Args &args, Map &map, Player &player, Inventory &inventory);

   // Members

   std::vector<std::string> output;
   Input input;
   bool shouldRender = false;
   bool fadingout = false;
   bool renderInGameState = false; // wtf
   int scrollback = 0;
   float outputDelay = 0.0f;
   float fadeoutTimer = 0.0f;
};

#endif
