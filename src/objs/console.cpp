#include "objs/console.hpp"
#include "mngr/resource.hpp"
#include "objs/inventory.hpp"
#include "util/format.hpp"
#include "util/parallax.hpp"
#include "util/render.hpp"
#include <functional>
#include <unordered_map>

// Constants

constexpr int maxLines = 7;
constexpr int consoleFontSize = 35;

static inline constexpr Color lineColors[(size_t)ConsoleColor::count] {
   WHITE, GRAY, YELLOW, RED, GREEN, BLUE, Color{255, 125, 0, 255}, Color{125, 0, 255, 255}, Color{255, 125, 255, 255}
};

// Commands

bool c_help(Console &console, const VArgs&, Map&, Player&, Inventory&) {
   console.output("Controls:", ConsoleColor::gray);
   console.output("UP - previous command from history.");
   console.output("DOWN - next command from history.");
   console.output("ENTER - run command.");
   console.output("ESC/CTRL+TAB - close.");
   console.output("Operators:", ConsoleColor::gray);
   console.output("& - execute next command only if the last was successful.");
   console.output("| - execute next command only if the last failed.");
   console.output("; - execute next command.");
   console.output("Commands:", ConsoleColor::gray);
   console.output("echo [MSG] - echo a message to the console.");
   console.output("hist - output command history.");
   console.output("chist - clear command history.");
   console.output("time [TIME] - set time of day.");
   console.output("stblk [ID/NAME] [X] [Y] - set block with the id/name at the given coordinates.");
   console.output("flblk [ID/NAME] [SX] [SY] [DX] [DY] - fill blocks with the id/name from coordinates (SX; SY) to (DX; DY).");
   console.output("stwl [ID/NAME] [X] [Y] - set wall with the id/name at the given coordinates.");
   console.output("flwl [ID/NAME] [SX] [SY] [DX] [DY] - fill walls with the id/name from coordinates (SX; SY) to (DX; DY).");
   console.output("stlq [ID/NAME] [X] [Y] - set liquid with the id/name at the given coordinates.");
   console.output("fllq [ID/NAME] [SX] [SY] [DX] [DY] - fill liquids with the id/name from coordinates (SX; SY) to (DX; DY).");
   console.output("tp [X] [Y] - teleport player to the given coordinates.");
   console.output("sp [X] [Y] - set player spawn point to the given coordinates.");
   console.output("crds - show current coordinates.");
   console.output("hp [HP] - set health.");
   console.output("maxhp [HP] - set maximum health.");
   console.output("kill - kill the player.");
   console.output("clear - clear the console.");
   console.output("exit - exit the console. Or simply press ESC!");
   console.output("Scroll back with the scroll wheel to see more commands.", ConsoleColor::gray);
   return true;
}

bool c_echo(Console &console, const VArgs &args, Map&, Player&, Inventory&) {
   if (args.size() != 2) {
      console.output("echo: expected exactly 1 argument.", ConsoleColor::red);
      return false;
   }
   console.output(args[1]);
   return true;
}

bool c_tp(Console &console, const VArgs &args, Map &map, Player &player, Inventory&) {
   if (args.size() != 3) {
      console.output("tp: expected exactly 2 arguments.", ConsoleColor::red);
      return false;
   }
   int x, y;

   // fuck this function and try blocks
   try {
      x = stoi(args[1]);
      y = stoi(args[2]);
   } catch (...) {
      console.output("tp: expected both arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("tp: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   player.maximumY = y; // Reset fall height for safety purposes
   player.ignoreCollision = true;
   player.position.x = x;
   player.position.y = y;
   console.output(TextFormat("tp: teleported to (X %d; Y %d).", x, y));
   return true;
}

bool c_sp(Console &console, const VArgs &args, Map &map, Player &player, Inventory&) {
   if (args.size() != 1 && args.size() != 3) {
      console.output("sp: expected 2 or no arguments.", ConsoleColor::red);
      return false;
   }
   int x, y;

   if (args.size() == 1) {
      x = player.position.x;
      y = player.position.y;
   } else {
      try {
         x = stoi(args[1]);
         y = stoi(args[2]);
      } catch (...) {
         console.output("sp: expected both arguments to be numbers.", ConsoleColor::red);
         return false;
      }
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("sp: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   player.spawnPos.x = x;
   player.spawnPos.y = y;
   console.output(TextFormat("sp: spawn position set to (X %d; Y %d).", x, y));
   return true;
}

bool c_crds(Console &console, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 1) {
      console.output("crds: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   console.output(TextFormat("crds: your position is (X %d; Y %d).", (int)player.position.x, (int)player.position.y));
   return true;
}

bool c_clear(Console &console, const VArgs&, Map&, Player&, Inventory&) {
   console.text.clear();
   console.text.shrink_to_fit(); // Clear memory too

   console.textColors.clear();
   console.textColors.shrink_to_fit();
   console.scrollback = 0;
   return true;
}

bool c_exit(Console &console, const VArgs&, Map&, Player&, Inventory&) {
   console.input.typing = false;
   return true;
}

bool c_quine(Console &console, const std::string &quine, const VArgs&, Map&, Player&, Inventory&) {
   console.output(quine);
   return true;
}

bool c_hp(Console &console, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 2) {
      console.output("hp: expected 1 argument.", ConsoleColor::red);
      return false;
   }

   int hp;
   try {
      hp = stoi(args[1]);
   } catch (...) {
      console.output("hp: expected first argument to be a number.", ConsoleColor::red);
      return false;
   }

   player.timeSinceLastDamage = player.timeSpentRegenerating = 0.0f;
   player.hearts = std::min(player.maxHearts, hp);
   console.output(TextFormat("hp: set health to %d.", hp));
   return true;
}

bool c_maxhp(Console &console, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 2) {
      console.output("maxhp: expected 1 argument.", ConsoleColor::red);
      return false;
   }

   int hp;
   try {
      hp = stoi(args[1]);
   } catch (...) {
      console.output("maxhp: expected first argument to be a number.", ConsoleColor::red);
      return false;
   }

   player.timeSinceLastDamage = player.timeSpentRegenerating = 0.0f;
   player.maxHearts = hp;
   player.hearts = std::min(player.maxHearts, player.hearts);
   console.output(TextFormat("maxhp: set maximum health to %d.", hp));
   return true;
}

bool c_br(Console &console, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 2) {
      console.output("br: expected 1 argument.", ConsoleColor::red);
      return false;
   }

   int br;
   try {
      br = stoi(args[1]);
   } catch (...) {
      console.output("br: expected first argument to be a number.", ConsoleColor::red);
      return false;
   }

   player.breath = br;
   console.output(TextFormat("br: set health to %d.", br));
   return true;
}

bool c_kill(Console &console, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 1) {
      console.output("kill: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   player.hearts = 0;
   console.output("kill: killed player.");
   return true;
}

bool c_time(Console &console, const VArgs &args, Map&, Player&, Inventory&) {
   if (args.size() != 2) {
      console.output("time: expected 1 argument.", ConsoleColor::red);
      return false;
   }

   float t;
   try {
      t = stof(args[1]) * (360.0f / 24.0f);
   } catch (...) {
      console.output("time: expected first argument to be a number.", ConsoleColor::red);
      return false;
   }

   setTimeOfDay(t);
   console.output(TextFormat("time: set time of day to %.2f.", t));
   return true;
}

bool c_hist(Console &console, const VArgs &args, Map&, Player&, Inventory&) {
   if (args.size() != 1) {
      console.output("hist: expected no arguments. Executing anyway.", ConsoleColor::red);
   }

   // Don't show the current 'hist' command in history
   for (size_t i = 0; i < console.history.size() - 1; ++i)
      console.output(TextFormat("%5lu: %s", i + 1, console.history[i].c_str()));
   return true;
}

bool c_chist(Console &console, const VArgs &args, Map&, Player&, Inventory&) {
   if (args.size() != 1) {
      console.output("chist: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   console.history.clear();
   console.history.shrink_to_fit();
   console.output("chist: history cleared.");
   return true;
}

bool c_stblk(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 4) {
      console.output("stblk: expected 3 argument.", ConsoleColor::red);
      return false;
   }

   int x, y, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("stblk: invalid block id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("stblk: expected first argument to either be a valid block id or name.", ConsoleColor::red);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("stblk: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("stblk: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   map.setBlock(x, y, id);
   console.output(TextFormat("stblk: set block at coordinates (X %d; Y %d) to '%s'.", x, y, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_flblk(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 6) {
      console.output("flblk: expected 5 argument.", ConsoleColor::red);
      return false;
   }

   int sx, sy, dx, dy, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("flblk: invalid block id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("flblk: expected first argument to either be a valid block id or name.", ConsoleColor::red);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      sx = stoi(args[2]);
      sy = stoi(args[3]);
      dx = stoi(args[4]);
      dy = stoi(args[5]);
   } catch (...) {
      console.output("flblk: expected second, third, fourth and fifth arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= map.sizeX || sy >= map.sizeY || dx < 0 || dy < 0 || dx >= map.sizeX || dy >= map.sizeY) {
      console.output("flblk: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         map.setBlock(x, y, id);
      }
   }
   console.output(TextFormat("flblk: filled all blocks from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_stwl(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 4) {
      console.output("stwl: expected 3 argument.", ConsoleColor::red);
      return false;
   }

   int x, y, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("stwl: invalid wall id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("stwl: expected first argument to either be a valid wall id or name.", ConsoleColor::red);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("stwl: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("stwl: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   map.setBlock(x, y, id, true);
   console.output(TextFormat("stwl: set wall at coordinates (X %d; Y %d) to '%s'.", x, y, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_flwl(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 6) {
      console.output("flwl: expected 5 argument.", ConsoleColor::red);
      return false;
   }

   int sx, sy, dx, dy, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("flwl: invalid wall id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("flwl: expected first argument to either be a valid wall id or name.", ConsoleColor::red);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      sx = stoi(args[2]);
      sy = stoi(args[3]);
      dx = stoi(args[4]);
      dy = stoi(args[5]);
   } catch (...) {
      console.output("flwl: expected second, third, fourth and fifth arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= map.sizeX || sy >= map.sizeY || dx < 0 || dy < 0 || dx >= map.sizeX || dy >= map.sizeY) {
      console.output("flwl: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         map.setBlock(x, y, id, true);
      }
   }
   console.output(TextFormat("flwl: filled all walls from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_stlq(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 4) {
      console.output("stlq: expected 3 argument.", ConsoleColor::red);
      return false;
   }

   // 0 -> none
   // 1 -> water
   // 2 -> lava
   // 3 -> honey
   int x, y, id;

   try {
      id = stoi(args[1]);

      if (id < 0 || id > 3) {
         console.output("stlq: invalid liquid id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      const static std::unordered_map<std::string, int> liquidIds {
         {"none", 0}, {"water", 1}, {"lava", 2}, {"honey", 3}
      };
      
      if (liquidIds.find(args[1]) == liquidIds.end()) {
         console.output("stlq: expected first argument to either be a valid liquid id or name.", ConsoleColor::red);
         return false;
      }
      id = liquidIds.at(args[1]);
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("stlq: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("stlq: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   map.liquidTypes[y][x] = (LiquidType)id;
   map.liquidsHeights[y][x] = (id == 0 ? 0 : maxLiquidLayers);;

   constexpr const char *liquidNames[] = {"none", "water", "lava", "honey"};
   console.output(TextFormat("stlq: set liquid at coordinates (X %d; Y %d) to '%s'.", x, y, liquidNames[id]));
   return true;
}

bool c_fllq(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 6) {
      console.output("fllq: expected 5 argument.", ConsoleColor::red);
      return false;
   }

   int sx, sy, dx, dy, id;
   try {
      id = stoi(args[1]);

      if (id < 0 || id > 3) {
         console.output("fllq: invalid liquid id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      const static std::unordered_map<std::string, int> liquidIds {
         {"none", 0}, {"water", 1}, {"lava", 2}, {"honey", 3}
      };
      
      if (liquidIds.find(args[1]) == liquidIds.end()) {
         console.output("fllq: expected first argument to either be a valid liquid id or name.", ConsoleColor::red);
         return false;
      }
      id = liquidIds.at(args[1]);
   }

   try {
      sx = stoi(args[2]);
      sy = stoi(args[3]);
      dx = stoi(args[4]);
      dy = stoi(args[5]);
   } catch (...) {
      console.output("fllq: expected second, third, fourth and fifth arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= map.sizeX || sy >= map.sizeY || dx < 0 || dy < 0 || dx >= map.sizeX || dy >= map.sizeY) {
      console.output("fllq: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         map.liquidTypes[y][x] = (LiquidType)id;
         map.liquidsHeights[y][x] = (id == 0 ? 0 : maxLiquidLayers);
      }
   }
   constexpr const char *liquidNames[] = {"none", "water", "lava", "honey"};
   console.output(TextFormat("fllq: filled all liquids from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, liquidNames[id]));
   return true;
}

// COMMAND MAP

using Command = std::function<bool(Console&, const VArgs&, Map&, Player&, Inventory&)>;
static inline const std::unordered_map<std::string, Command> commands {
   {"help", c_help},
   {"echo", c_echo},
   {"tp", c_tp},
   {"sp", c_sp},
   {"crds", c_crds},
   {"clear", c_clear},
   {"exit", c_exit},
   {"hp", c_hp},
   {"maxhp", c_maxhp},
   {"br", c_br},
   {"kill", c_kill},
   {"time", c_time},
   {"hist", c_hist},
   {"chist", c_chist},
   {"stblk", c_stblk},
   {"flblk", c_flblk},
   {"stwl", c_stwl},
   {"flwl", c_flwl},
   {"stlq", c_stlq},
   {"fllq", c_fllq},
};

// init

void Console::init() {
   input.rectangle = {500.0f, GetScreenHeight() - 25.0f, 1000.0f, 50.0f};
   input.fallback = "'help' for a list of commands.";
   input.wrapinput = false;
   input.maxChars = 512;
}

void Console::output(const std::string &string, ConsoleColor color) {
   size_t last = text.size();
   divideText(text, string, input.rectangle.width - 10.0f, 35, 1.0f);

   for (size_t i = last; i < text.size(); ++i) {
      textColors.push_back(color);
   }
}

// Update

void Console::update(Map &map, Player &player, Inventory &inventory) {
   bool wastyping = input.typing;
   input.update();

   if (wastyping && !input.typing && IsKeyPressed(KEY_ENTER)) {
      input.typing = true;
      lex(map, player, inventory);
   }

   if (!history.empty() && (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))) {
      historyIndex = (historyIndex == 0 ? history.size() - 1 : historyIndex - 1);
      input.text = history[historyIndex];
   } else if (!history.empty() && (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))) {
      historyIndex = (historyIndex + 1) % history.size();
      input.text = history[historyIndex];
   }

   if (input.changed) {
      historyIndex = 0;
   }

   if (input.typing) {
      float thing = GetMouseWheelMove();
      if (thing >= 1.0f) {
         scrollback = std::max(0, scrollback - 1);
      } else if (thing <= -1.0f) {
         scrollback = std::min(std::max(0, (int)text.size() - maxLines), scrollback + 1);
      }
   }
}

// Render

void Console::render() {
   if (!input.typing) return;
   drawRect(input.rectangle, Fade(BLACK, 0.9f));
   input.render();

   Font &font = getFont("andy");
   float ym125 = input.rectangle.y - 125.0f;
   float hp250 = input.rectangle.height + 250.0f;
   float xconst = input.rectangle.x - input.rectangle.width / 2.0f + 5.0f;
   float yconst = ym125 - hp250 / 2.0f;

   drawRect({input.rectangle.x, ym125 - input.rectangle.height, input.rectangle.width, hp250}, Fade(BLACK, 0.75f));
   for (int i = scrollback; i < scrollback + maxLines && (size_t)i < text.size(); ++i) {
      DrawTextPro(font, text[i].c_str(), {xconst, yconst + (i - scrollback - 1) * 40.0f}, {0, 0}, 0, consoleFontSize, 1, lineColors[(size_t)textColors[i]]);
   }
}

// Lexing/command logic

void Console::lex(Map &map, Player &player, Inventory &inventory) {
   history.push_back(input.text);
   size_t index = 0;
   VArgs args;

   for (; index < input.text.size(); ++index) {
      char ch = input.text[index];

      if (ch == ';') {
         if (args.empty()) {
            output("operator ';': no command to execute.", ConsoleColor::red);
            goto QUIT_LEXING;
         }

         handleCommand(args, map, player, inventory);
         args.clear();
      } else if (ch == '&') {
         if (args.empty()) {
            output("operator '&': no command to execute.", ConsoleColor::red);
            goto QUIT_LEXING;
         }
         
         if (!handleCommand(args, map, player, inventory)) goto QUIT_LEXING;
         args.clear();
      } else if (ch == '|') {
         if (args.empty()) {
            output("operator '|': no command to execute.", ConsoleColor::red);
            goto QUIT_LEXING;
         }

         if (handleCommand(args, map, player, inventory)) goto QUIT_LEXING;
         args.clear();
      } else if (ch == '"') {
         std::string str;
         index += 1;
         if (index >= input.text.size()) {
            output("operator '\"': unterminated string.", ConsoleColor::red);
            goto QUIT_LEXING;
         }

         for (ch = input.text[index]; index < input.text.size() && ch != '"'; ch = input.text[++index])
            str.push_back(ch);

         if (index >= input.text.size() || ch != '"') {
            output("operator '\"': unterminated string.", ConsoleColor::red);
            goto QUIT_LEXING;
         }
         args.push_back(str);
      } else if (std::isspace(ch)) {
         continue;
      } else {
         std::string arg;

         for (ch = input.text[index]; index < input.text.size() && !std::isspace(ch); ch = input.text[++index]) {
            if (ch == '&' || ch == '|' || ch == ';') {
               index -= 1;
               break;
            }
            arg.push_back(ch);
         }
         args.push_back(arg);
      }
   }
   handleCommand(args, map, player, inventory);

QUIT_LEXING:
   input.text.clear();
   scrollback = std::max(0, (int)text.size() - maxLines);
   historyIndex = 0;
}

bool Console::handleCommand(VArgs &args, Map &map, Player &player, Inventory &inventory) {
   if (auto it = commands.find(args[0]); it != commands.end()) {
      return it->second(*this, args, map, player, inventory);
   } else {
      output("Invalid command. See 'help' for a list of commands.", ConsoleColor::red);
      return false;
   }
}
