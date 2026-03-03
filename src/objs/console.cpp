#include "objs/console.hpp"
#include "mngr/resource.hpp"
#include "objs/inventory.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <functional>
#include <sstream>
#include <unordered_map>

// Constants

constexpr int maxVisibleLinesOnScreenAtOnce = 6;

using Command = std::function<bool(ArgsList)>;
static inline const std::unordered_map<std::string, Command> commands {
   {"help", c_help},
   {"tp", c_tp},
   {"sp", c_sp},
   {"crds", c_crds},
   {"clear", c_clear},
   {"exit", c_exit},
   {"hp", c_hp},
   {"maxhp", c_maxhp},
   {"br", c_br},
   {"kill", c_kill},
};

static inline constexpr Color lineColors[(size_t)ConsoleColor::count] {
   WHITE, GRAY, YELLOW, RED, GREEN, BLUE, Color{255, 125, 0, 255}, Color{125, 0, 255, 255}, Color{255, 125, 255, 255}
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

void Console::update(Map &map, Player &player, Inventory &inventory) {
   bool wastyping = input.typing;
   input.update();

   if (wastyping && !input.typing && IsKeyPressed(KEY_ENTER)) {
      input.typing = true;
      lex(map, player, inventory);
   }

   if (input.typing) {
      float thing = GetMouseWheelMove();
      if (thing >= 1.0f) {
         scrollback = std::max(0, scrollback - 1);
      } else if (thing <= -1.0f) {
         scrollback = std::min(std::max(0, (int)text.size() - maxVisibleLinesOnScreenAtOnce), scrollback + 1);
      }
   }
}

void Console::render() {
   if (!input.typing) return;
   drawRect(input.rectangle, Fade(BLACK, 0.9f));
   input.render();
   drawRect({input.rectangle.x, input.rectangle.y - 125.0f - input.rectangle.height, input.rectangle.width, input.rectangle.height + 250.0f}, Fade(BLACK, 0.75f));

   for (int i = scrollback; i < scrollback + maxVisibleLinesOnScreenAtOnce && (size_t)i < text.size(); ++i) {
      DrawTextPro(getFont("andy"), text[i].c_str(), {input.rectangle.x - input.rectangle.width / 2.0f + 5.0f, (input.rectangle.y - 125.0f) - (input.rectangle.height + 250.0f) / 2.0f + (i - scrollback) * 40}, {0, getOrigin(text[i].c_str(), 35, 1).y}, 0, 35, 1, lineColors[(size_t)textColors[i]]);
   }
}

// Commands

void Console::lex(Map &map, Player &player, Inventory &inventory) {
   std::stringstream s (input.text);
   std::string temp;
   VArgs args;

   while (s >> temp) {
      args.push_back(temp);
   }

   if (args.empty()) {
      return;
   }

   handleCommand(args, map, player, inventory);
   input.text.clear();
   scrollback = std::max(0, (int)text.size() - maxVisibleLinesOnScreenAtOnce);
}

bool Console::handleCommand(const VArgs &args, Map &map, Player &player, Inventory &inventory) {
   if (auto it = commands.find(args[0]); it != commands.end()) {
      return it->second(*this, input.text, args, map, player, inventory);
   } else {
      output("Invalid command. See 'help' for a list of commands.", ConsoleColor::red);
      return false;
   }
}

bool c_help(Console &console, const std::string&, const VArgs&, Map&, Player&, Inventory&) {
   console.output("Operators:", ConsoleColor::gray);
   console.output("Commands:", ConsoleColor::gray);
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

bool c_tp(Console &console, const std::string&, const VArgs &args, Map &map, Player &player, Inventory&) {
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

bool c_sp(Console &console, const std::string&, const VArgs &args, Map &map, Player &player, Inventory&) {
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

bool c_crds(Console &console, const std::string&, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 1) {
      console.output("crds: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   console.output(TextFormat("crds: your position is (X %d; Y %d).", (int)player.position.x, (int)player.position.y));
   return true;
}

bool c_clear(Console &console, const std::string&, const VArgs&, Map&, Player&, Inventory&) {
   console.text.clear();
   console.text.shrink_to_fit(); // Clear memory too

   console.textColors.clear();
   console.textColors.shrink_to_fit();
   console.scrollback = 0;
   return true;
}

bool c_exit(Console &console, const std::string&, const VArgs&, Map&, Player&, Inventory&) {
   console.input.typing = false;
   return true;
}

bool c_quine(Console &console, const std::string &quine, const VArgs&, Map&, Player&, Inventory&) {
   console.output(quine);
   return true;
}

bool c_hp(Console &console, const std::string&, const VArgs &args, Map&, Player &player, Inventory&) {
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

bool c_maxhp(Console &console, const std::string&, const VArgs &args, Map&, Player &player, Inventory&) {
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

bool c_br(Console &console, const std::string&, const VArgs &args, Map&, Player &player, Inventory&) {
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

bool c_kill(Console &console, const std::string&, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 1) {
      console.output("kill: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   player.hearts = 0;
   console.output("kill: killed player.");
   return true;
}
