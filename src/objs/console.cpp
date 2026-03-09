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
   console.output("place [ID/NAME] [X] [Y] - set block with the id/name at the given coordinates.");
   console.output("fill [ID/NAME] [SX] [SY] [DX] [DY] - fill blocks with the id/name from coordinates (SX; SY) to (DX; DY).");
   console.output("placew [ID/NAME] [X] [Y] - set wall with the id/name at the given coordinates.");
   console.output("fillw [ID/NAME] [SX] [SY] [DX] [DY] - fill walls with the id/name from coordinates (SX; SY) to (DX; DY).");
   console.output("placeq [ID/NAME] [X] [Y] - set liquid with the id/name at the given coordinates.");
   console.output("fillq [ID/NAME] [SX] [SY] [DX] [DY] - fill liquids with the id/name from coordinates (SX; SY) to (DX; DY).");
   console.output("give [i/b/e/p] [ID] [COUNT] - give item of specified id and quantity to the player.");
   console.output("set [VAR] [VALUE] - set VAR to VALUE.");
   console.output("list - list all variables.");
   console.output("cinv - clear the inventory.");
   console.output("tp [X] [Y] - teleport player to the given coordinates.");
   console.output("spawnpoint [X] [Y] - set player spawn point to the given coordinates.");
   console.output("pos - show current coordinates.");
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

bool c_spawnpoint(Console &console, const VArgs &args, Map &map, Player &player, Inventory&) {
   if (args.size() != 1 && args.size() != 3) {
      console.output("spawnpoint: expected 2 or no arguments.", ConsoleColor::red);
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
         console.output("spawnpoint: expected both arguments to be numbers.", ConsoleColor::red);
         return false;
      }
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("spawnpoint: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   player.spawnPos.x = x;
   player.spawnPos.y = y;
   console.output(TextFormat("spawnpoint: spawn position set to (X %d; Y %d).", x, y));
   return true;
}

bool c_pos(Console &console, const VArgs &args, Map&, Player &player, Inventory&) {
   if (args.size() != 1) {
      console.output("pos: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   console.output(TextFormat("pos: your position is (X %d; Y %d).", (int)player.position.x, (int)player.position.y));
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

bool c_place(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 4) {
      console.output("place: expected 3 arguments.", ConsoleColor::red);
      return false;
   }

   int x, y, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("place: invalid block id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("place: expected first argument to either be a valid block id or name.", ConsoleColor::red);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("place: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("place: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   map.setBlock(x, y, id);
   console.output(TextFormat("place: set block at coordinates (X %d; Y %d) to '%s'.", x, y, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_fill(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 6) {
      console.output("fill: expected 5 arguments.", ConsoleColor::red);
      return false;
   }

   int sx, sy, dx, dy, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("fill: invalid block id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("fill: expected first argument to either be a valid block id or name.", ConsoleColor::red);
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
      console.output("fill: expected second, third, fourth and fifth arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= map.sizeX || sy >= map.sizeY || dx < 0 || dy < 0 || dx >= map.sizeX || dy >= map.sizeY) {
      console.output("fill: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         map.setBlock(x, y, id);
      }
   }
   console.output(TextFormat("fill: filled all blocks from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_placew(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 4) {
      console.output("placew: expected 3 arguments.", ConsoleColor::red);
      return false;
   }

   int x, y, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("placew: invalid wall id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("placew: expected first argument to either be a valid wall id or name.", ConsoleColor::red);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("placew: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("placew: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   map.setBlock(x, y, id, true);
   console.output(TextFormat("placew: set wall at coordinates (X %d; Y %d) to '%s'.", x, y, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_fillw(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 6) {
      console.output("fillw: expected 5 arguments.", ConsoleColor::red);
      return false;
   }

   int sx, sy, dx, dy, id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("fillw: invalid wall id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("fillw: expected first argument to either be a valid wall id or name.", ConsoleColor::red);
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
      console.output("fillw: expected second, third, fourth and fifth arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= map.sizeX || sy >= map.sizeY || dx < 0 || dy < 0 || dx >= map.sizeX || dy >= map.sizeY) {
      console.output("fillw: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         map.setBlock(x, y, id, true);
      }
   }
   console.output(TextFormat("fillw: filled all walls from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_placeq(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 4) {
      console.output("placeq: expected 3 arguments.", ConsoleColor::red);
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
         console.output("placeq: invalid liquid id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      const static std::unordered_map<std::string, int> liquidIds {
         {"none", 0}, {"water", 1}, {"lava", 2}, {"honey", 3}
      };
      
      if (liquidIds.find(args[1]) == liquidIds.end()) {
         console.output("placeq: expected first argument to either be a valid liquid id or name.", ConsoleColor::red);
         return false;
      }
      id = liquidIds.at(args[1]);
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("placeq: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      console.output("placeq: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   map.deleteBlock(x, y);
   map.liquidTypes[y][x] = (LiquidType)id;
   map.liquidsHeights[y][x] = (id == 0 ? 0 : maxLiquidLayers);

   constexpr const char *liquidNames[] = {"none", "water", "lava", "honey"};
   console.output(TextFormat("placeq: set liquid at coordinates (X %d; Y %d) to '%s'.", x, y, liquidNames[id]));
   return true;
}

bool c_fillq(Console &console, const VArgs &args, Map &map, Player&, Inventory&) {
   if (args.size() != 6) {
      console.output("fillq: expected 5 arguments.", ConsoleColor::red);
      return false;
   }

   int sx, sy, dx, dy, id;
   try {
      id = stoi(args[1]);

      if (id < 0 || id > 3) {
         console.output("fillq: invalid liquid id.", ConsoleColor::red);
         return false;
      }
   } catch (...) {
      const static std::unordered_map<std::string, int> liquidIds {
         {"none", 0}, {"water", 1}, {"lava", 2}, {"honey", 3}
      };
      
      if (liquidIds.find(args[1]) == liquidIds.end()) {
         console.output("fillq: expected first argument to either be a valid liquid id or name.", ConsoleColor::red);
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
      console.output("fillq: expected second, third, fourth and fifth arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= map.sizeX || sy >= map.sizeY || dx < 0 || dy < 0 || dx >= map.sizeX || dy >= map.sizeY) {
      console.output("fillq: coordinates are out of bounds.", ConsoleColor::red);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         map.deleteBlock(x, y);
         map.liquidTypes[y][x] = (LiquidType)id;
         map.liquidsHeights[y][x] = (id == 0 ? 0 : maxLiquidLayers);
      }
   }
   constexpr const char *liquidNames[] = {"none", "water", "lava", "honey"};
   console.output(TextFormat("fillq: filled all liquids from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, liquidNames[id]));
   return true;
}

bool c_give(Console &console, const VArgs &args, Map&, Player &player, Inventory &inventory) {
   if (args.size() != 4 && args.size() != 3) {
      console.output("give: expected 2 or 3 arguments.", ConsoleColor::red);
      return false;
   }

   Item item;
   int id, count = 1;

   try {
      if (args[1].size() != 1) {
         console.output("give: invalid first argument, expected item type - b/i/e/p.", ConsoleColor::red);
         return false;
      }

      id = stoi(args[2]);
      if (args.size() == 4) {
         count = stoi(args[3]);
      }

      if (count < 0 || count > 9999) {
         console.output("give: invalid item count.", ConsoleColor::red);
         return false;
      }

      switch (args[1][0]) {
      case 'b':
         if (id < 0 || id >= getBlockCount()) {
            console.output("give: invalid block id.", ConsoleColor::red);
            return false;
         }
         item.type = ItemType::block;
         break;
      case 'i':
         if (id < 0 || id >= (int)getItemCount()) {
            console.output("give: invalid item id.", ConsoleColor::red);
            return false;
         }
         item.type = ItemType::item;
         break;
      case 'e':
         if (id < 0 || id >= (int)getToolCount()) {
            console.output("give: invalid equipment id.", ConsoleColor::red);
            return false;
         }
         item.type = ItemType::equipment;
         break;
      case 'p':
         if (id < 0 || id >= (int)getPotionCount()) {
            console.output("give: invalid potion id.", ConsoleColor::red);
            return false;
         }
         item.type = ItemType::potion;
         break;
      default:
         console.output("give: invalid first argument, expected item type - b/i/e/p.", ConsoleColor::red);
         return false;
      }

      item.count = count;
      item.id = id;
   } catch (...) {
      console.output("give: expected second and third arguments to be numbers.", ConsoleColor::red);
      return false;
   }

   inventory.tryToPlaceItemOrDropAtCoordinates(item, player.position.x, player.position.y);
   console.output(TextFormat("give: gave %d of item id %d.", count, id));
   return true;
}

bool c_cinv(Console &console, const VArgs &args, Map&, Player&, Inventory &inventory) {
   if (args.size() != 1) {
      console.output("cinv: expected no arguments. Executing anyway.", ConsoleColor::red);
   }
   for (int y = 0; y < inventoryHeight; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         inventory.items[y][x] = Item{};
      }
   }
   inventory.trashedItem = Item{};
   inventory.anySelected = false;
   inventory.selectedItem.reset();
   return true;
}

bool c_set(Console &console, const VArgs &args, Map&, Player&, Inventory&) {
   if (args.size() != 3) {
      console.output("set: expected 2 arguments.", ConsoleColor::red);
      return false;
   }
   
   if (console.vars.find(args[1]) == console.vars.end()) {
      console.output(TextFormat("set: unknown variable '%s'.", args[1].c_str()), ConsoleColor::red);
      return false;
   }

   float value;
   try {
      value = stof(args[2]);
   } catch (...) {
      console.output("set: expected second argument to be a number.", ConsoleColor::red);
      return false;
   }

   if (auto *b = std::get_if<bool*>(&console.vars[args[1]])) {
      **b = value != 0.0f;
   } else if (auto *i = std::get_if<int*>(&console.vars[args[1]])) {
      **i = (int)value;
   } else if (auto *f = std::get_if<float*>(&console.vars[args[1]])) {
      **f = value;
   }
   console.output(TextFormat("set: set variable '%s' to '%0.f'.", args[1].c_str(), value));
   return true;
}

bool c_list(Console &console, const VArgs &args, Map&, Player&, Inventory&) {
   if (args.size() != 1) {
      console.output("list: expected no arguments. Executing anyway.", ConsoleColor::red);
   }

   console.output("Variables:", ConsoleColor::gray);
   for (auto &[key, value] : console.vars) {
      std::string msg = key + ": ";
      if (auto *b = std::get_if<bool*>(&value)) {
         msg += std::to_string(**b);
      } else if (auto *i = std::get_if<int*>(&value)) {
         msg += std::to_string(**i);
      } else if (auto *f = std::get_if<float*>(&value)) {
         msg += std::to_string(**f);
      }
      console.output(msg);
   }
   return true;
}

// COMMAND MAP

using Command = std::function<bool(Console&, const VArgs&, Map&, Player&, Inventory&)>;
static inline const std::unordered_map<std::string, Command> commands {
   {"help", c_help},
   {"echo", c_echo},
   {"tp", c_tp},
   {"spawnpoint", c_spawnpoint},
   {"pos", c_pos},
   {"clear", c_clear},
   {"cinv", c_cinv},
   {"exit", c_exit},
   {"hp", c_hp},
   {"maxhp", c_maxhp},
   {"kill", c_kill},
   {"time", c_time},
   {"hist", c_hist},
   {"chist", c_chist},
   {"place", c_place},
   {"fill", c_fill},
   {"placew", c_placew},
   {"fillw", c_fillw},
   {"placeq", c_placeq},
   {"fillq", c_fillq},
   {"give", c_give},
   {"set", c_set},
   {"list", c_list},
};

// init

void Console::init(Map &map, Player &player, Inventory &inventory) {
   input.rectangle = {500.0f, GetScreenHeight() - 25.0f, 1000.0f, 50.0f};
   input.fallback = "'help' for a list of commands.";
   input.wrapinput = false;
   input.maxChars = 512;

   // I might be crazy
   // Player
   vars["position.x"] = Variable(&player.position.x);
   vars["position.y"] = Variable(&player.position.y);
   vars["spawnPos.x"] = Variable(&player.spawnPos.x);
   vars["spawnPos.y"] = Variable(&player.spawnPos.y);
   vars["velocity.x"] = Variable(&player.velocity.x);
   vars["velocity.y"] = Variable(&player.velocity.y);
   vars["previousPosition.x"] = Variable(&player.previousPosition.x);
   vars["previousPosition.y"] = Variable(&player.previousPosition.y);
   vars["delta.x"] = Variable(&player.delta.x);
   vars["delta.y"] = Variable(&player.delta.y);
   vars["blockInput"] = Variable(&player.blockInput);
   vars["feetCollision"] = Variable(&player.feetCollision);
   vars["torsoCollision"] = Variable(&player.torsoCollision);
   vars["feetCollisionY"] = Variable(&player.feetCollisionY);
   vars["onGround"] = Variable(&player.onGround);
   vars["shouldBounce"] = Variable(&player.shouldBounce);
   vars["coyoteTimer"] = Variable(&player.coyoteTimer);
   vars["foxTimer"] = Variable(&player.foxTimer);
   vars["maximumY"] = Variable(&player.maximumY);
   vars["waterMultiplier"] = Variable(&player.waterMultiplier);
   vars["iceMultiplier"] = Variable(&player.iceMultiplier);
   vars["fallTimer"] = Variable(&player.fallTimer);
   vars["walkTimer"] = Variable(&player.walkTimer);
   vars["jumpTimer"] = Variable(&player.jumpTimer);
   vars["walkFrame"] = Variable(&player.walkFrame);
   vars["frameX"] = Variable(&player.frameX);
   vars["flipX"] = Variable(&player.flipX);
   vars["sitting"] = Variable(&player.sitting);
   vars["ignoreCollision"] = Variable(&player.ignoreCollision);
   vars["breathFrameCounter"] = Variable(&player.breathFrameCounter);
   vars["breath"] = Variable(&player.breath);
   vars["lastHearts"] = Variable(&player.lastHearts);
   vars["hearts"] = Variable(&player.hearts);
   vars["maxHearts"] = Variable(&player.maxHearts);
   vars["regenerationFrameCounter"] = Variable(&player.regenerationFrameCounter);
   vars["immunityFrame"] = Variable(&player.immunityFrame);
   vars["timeSinceLastDamage"] = Variable(&player.timeSinceLastDamage);
   vars["timeSpentRegenerating"] = Variable(&player.timeSpentRegenerating);
   vars["regenSpeedMultiplier"] = Variable(&player.regenSpeedMultiplier);
   vars["regeneration"] = Variable(&player.regeneration);
   vars["displayHearts"] = Variable(&player.displayHearts);
   vars["displayBreath"] = Variable(&player.displayBreath);
   vars["breakAnimation"] = Variable(&player.breakAnimation);
   vars["lastBreakingX"] = Variable(&player.lastBreakingX);
   vars["lastBreakingY"] = Variable(&player.lastBreakingY);
   vars["breakingWall"] = Variable(&player.breakingWall);
   vars["breakingFurniture"] = Variable(&player.breakingFurniture);
   vars["placedBlock"] = Variable(&player.placedBlock);
   vars["breakingBlock"] = Variable(&player.breakingBlock);
   vars["breakTime"] = Variable(&player.breakTime);
   vars["breakAnimationTimer"] = Variable(&player.breakAnimationTimer);
   vars["creative"] = Variable(&player.creative);

   // Map
   vars["map.size.x"] = Variable(&map.sizeX);
   vars["map.size.y"] = Variable(&map.sizeY);
   vars["lightingEnabled"] = Variable(&map.lightingEnabled);
   vars["waterShaderEnabled"] = Variable(&map.waterShaderEnabled);

   // Inventory
   vars["inventory.selected.x"] = Variable(&inventory.selectedX);
   vars["inventory.selected.y"] = Variable(&inventory.selectedY);
   vars["inventory.open"] = Variable(&inventory.open);
   vars["inventory.anySelected"] = Variable(&inventory.anySelected);
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
   if (input.text.empty()) {
      return;
   }
   
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
      } else if (ch == '$') {
         std::string var;
         index += 1;
         if (index >= input.text.size()) {
            output("operator '$': no variable present.", ConsoleColor::red);
            goto QUIT_LEXING;
         }

         for (ch = input.text[index]; index < input.text.size() && (std::isalnum(ch) || ch == '.'); ch = input.text[++index])
            var.push_back(ch);

         if (vars.find(var) == vars.end()) {
            output(TextFormat("operator '$': no such variable '%s'.", var.c_str()), ConsoleColor::red);
            goto QUIT_LEXING;
         }
         
         // Classic c++ boilerplate
         std::string str;
         if (auto *b = std::get_if<bool*>(&vars[var])) {
            str = std::to_string(**b);
         } else if (auto *i = std::get_if<int*>(&vars[var])) {
            str = std::to_string(**i);
         } else if (auto *f = std::get_if<float*>(&vars[var])) {
            str = std::to_string(**f);
         }
         args.push_back(str);
         index -= 1;
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
            if (ch == '&' || ch == '|' || ch == ';' || ch == '$') {
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
   if (args.empty()) {
      return false;
   }
   
   if (auto it = commands.find(args[0]); it != commands.end()) {
      return it->second(*this, args, map, player, inventory);
   } else {
      output("Invalid command. See 'help' for a list of commands.", ConsoleColor::red);
      return false;
   }
}
