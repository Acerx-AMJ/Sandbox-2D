#pragma once
#include "ui/input.hpp"
#include <unordered_map>
#include <vector>

enum class VariableType: unsigned char {
   none, boolean, integer, floating, string
};

struct Variable {
   VariableType type = VariableType::none;
   union {
      bool *bvalue;
      int *ivalue;
      float *fvalue;
      std::string *svalue;
   };
};

Variable createVariable(bool *value);
Variable createVariable(int *value);
Variable createVariable(float *value);
Variable createVariable(std::string *value);

struct Console {
   void init(struct GameState &state);
   void update(float dt, struct GameState &state);
   void updateResponsiveness();
   void render();

   // Commands

   void output(const std::string &string, Color color = WHITE);
   void lex(struct GameState &state);
   bool handleCommand(std::vector<std::string> &args, struct GameState &state);

   // Members

   std::unordered_map<std::string, Variable> vars;
   std::vector<std::string> text, history;
   std::vector<Color> textColors;

   Input input;
   int scrollback = 0;
   int historyIndex = 0;
};
