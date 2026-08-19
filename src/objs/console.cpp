#include "game/gameState.hpp"
#include "objs/console.hpp"
#include "objs/inventory.hpp"
#include "objs/parallax.hpp"
#include "objs/player.hpp"
#include "SRU/assets.hpp"
#include "SRU/file.hpp"
#include "SRU/render.hpp"
#include "SRU/text.hpp"
#include "SRU/util.hpp"
#include <unordered_map>

// Constants

constexpr int maxLines = 7;
constexpr int consoleFontSize = 35;

// Variables

Variable createVariable(bool *value) {
   Variable var {VariableType::boolean};
   var.bvalue = value;
   return var;
}

Variable createVariable(int *value) {
   Variable var {VariableType::integer};
   var.ivalue = value;
   return var;
}

Variable createVariable(float *value) {
   Variable var {VariableType::floating};
   var.fvalue = value;
   return var;
}

Variable createVariable(std::string *value) {
   Variable var {VariableType::string};
   var.svalue = value;
   return var;
}

// Commands

bool c_help(Console &console, std::vector<std::string> &args, GameState&) {
   console.output("Controls:", GRAY);
   console.output("UP - previous command from history.");
   console.output("DOWN - next command from history.");
   console.output("ENTER - run command.");
   console.output("ESC/CTRL+TAB - close.");
   console.output("Operators:", GRAY);
   console.output("& - execute next command only if the last was successful.");
   console.output("| - execute next command only if the last failed.");
   console.output("; - execute next command.");
   console.output("Commands:", GRAY);
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
   console.output("placef [ID/NAME] [X] [Y] - attempt to place specified furniture at coordinates (X; Y).");
   console.output("give [NAME] [COUNT] - give specified item to the player.");
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
   console.output("Scroll back with the scroll wheel to see more commands.", GRAY);
   return true;
}

bool c_echo(Console &console, std::vector<std::string> &args, GameState&) {
   args.erase(args.begin());
   std::string message = join(args);
   console.output(message);
   return true;
}

bool c_tp(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 3) {
      console.output("tp: expected exactly 2 arguments.", RED);
      return false;
   }

   int x, y;
   try {
      x = stoi(args[1]);
      y = stoi(args[2]);
   } catch (...) {
      console.output("tp: expected both arguments to be numbers.", RED);
      return false;
   }

   if (x < 0 || y < 0 || x >= state.map.sizeX || y >= state.map.sizeY) {
      console.output("tp: coordinates are out of bounds.", RED);
      return false;
   }

   state.player.maximumY = y; // Reset fall height for safety purposes
   state.player.ignoreCollision = true;
   state.player.position.x = x;
   state.player.position.y = y;
   console.output(TextFormat("tp: teleported to (X %d; Y %d).", x, y));
   return true;
}

bool c_spawnpoint(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 1 && args.size() != 3) {
      console.output("spawnpoint: expected 2 or no arguments.", RED);
      return false;
   }

   int x, y;
   if (args.size() == 1) {
      x = state.player.position.x;
      y = state.player.position.y;
   } else {
      try {
         x = stoi(args[1]);
         y = stoi(args[2]);
      } catch (...) {
         console.output("spawnpoint: expected both arguments to be numbers.", RED);
         return false;
      }
   }

   if (x < 0 || y < 0 || x >= state.map.sizeX || y >= state.map.sizeY) {
      console.output("spawnpoint: coordinates are out of bounds.", RED);
      return false;
   }

   state.player.spawnPos.x = x;
   state.player.spawnPos.y = y;
   console.output(TextFormat("spawnpoint: spawn position set to (X %d; Y %d).", x, y));
   return true;
}

bool c_pos(Console &console, std::vector<std::string> &args, GameState &state) {
   console.output(TextFormat("pos: your position is (X %d; Y %d).", (int)state.player.position.x, (int)state.player.position.y));
   return true;
}

bool c_clear(Console &console, std::vector<std::string> &args, GameState &state) {
   console.text.clear();
   console.text.shrink_to_fit(); // Clear memory too

   console.textColors.clear();
   console.textColors.shrink_to_fit();
   console.scrollback = 0;
   return true;
}

bool c_exit(Console &console, std::vector<std::string> &args, GameState &state) {
   console.input.typing = false;
   return true;
}

bool c_hp(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 2) {
      console.output("hp: expected 1 argument.", RED);
      return false;
   }

   int hp;
   try {
      hp = stoi(args[1]);
   } catch (...) {
      console.output("hp: expected first argument to be a number.", RED);
      return false;
   }

   state.player.timeSinceLastDamage = state.player.timeSpentRegenerating = 0.0f;
   state.player.hearts = std::min(state.player.maxHearts, hp);
   console.output(TextFormat("hp: set health to %d.", hp));
   return true;
}

bool c_maxhp(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 2) {
      console.output("maxhp: expected 1 argument.", RED);
      return false;
   }

   int hp;
   try {
      hp = stoi(args[1]);
   } catch (...) {
      console.output("maxhp: expected first argument to be a number.", RED);
      return false;
   }

   state.player.timeSinceLastDamage = state.player.timeSpentRegenerating = 0.0f;
   state.player.maxHearts = hp;
   state.player.hearts = std::min(state.player.maxHearts, state.player.hearts);
   console.output(TextFormat("maxhp: set maximum health to %d.", hp));
   return true;
}

bool c_kill(Console &console, std::vector<std::string> &args, GameState &state) {
   state.player.hearts = 0;
   console.output("kill: killed player.");
   return true;
}

bool c_time(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 2) {
      console.output("time: expected 1 argument.", RED);
      return false;
   }

   float t;
   try {
      t = (stof(args[1]) - 7.0f) * (360.0f / 24.0f);
   } catch (...) {
      console.output("time: expected first argument to be a number.", RED);
      return false;
   }

   setTimeOfDay(t);
   console.output(TextFormat("time: set time of day to %.2f.", t));
   return true;
}

bool c_hist(Console &console, std::vector<std::string> &args, GameState &state) {
   // Don't show the current 'hist' command in history
   for (size_t i = 0; i < console.history.size() - 1; ++i)
      console.output(TextFormat("%5lu: %s", i + 1, console.history[i].c_str()));
   return true;
}

bool c_chist(Console &console, std::vector<std::string> &args, GameState &state) {
   console.history.clear();
   console.history.shrink_to_fit();
   console.output("chist: history cleared.");
   return true;
}

bool c_place(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 4) {
      console.output("place: expected 3 arguments.", RED);
      return false;
   }

   int x, y;
   blockid_t id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("place: invalid block id.", RED);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("place: expected first argument to either be a valid block id or name.", RED);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("place: expected second and third arguments to be numbers.", RED);
      return false;
   }

   if (x < 0 || y < 0 || x >= state.map.sizeX || y >= state.map.sizeY) {
      console.output("place: coordinates are out of bounds.", RED);
      return false;
   }

   state.map.setBlock(x, y, id);
   console.output(TextFormat("place: set block at coordinates (X %d; Y %d) to '%s'.", x, y, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_fill(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 6) {
      console.output("fill: expected 5 arguments.", RED);
      return false;
   }

   int sx, sy, dx, dy;
   blockid_t id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("fill: invalid block id.", RED);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("fill: expected first argument to either be a valid block id or name.", RED);
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
      console.output("fill: expected second, third, fourth and fifth arguments to be numbers.", RED);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= state.map.sizeX || sy >= state.map.sizeY || dx < 0 || dy < 0 || dx >= state.map.sizeX || dy >= state.map.sizeY) {
      console.output("fill: coordinates are out of bounds.", RED);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         state.map.setBlock(x, y, id);
      }
   }
   console.output(TextFormat("fill: filled all blocks from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_placew(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 4) {
      console.output("placew: expected 3 arguments.", RED);
      return false;
   }

   int x, y;
   blockid_t id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("placew: invalid wall id.", RED);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("placew: expected first argument to either be a valid wall id or name.", RED);
         return false;
      }
      id = getBlockIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("placew: expected second and third arguments to be numbers.", RED);
      return false;
   }

   if (x < 0 || y < 0 || x >= state.map.sizeX || y >= state.map.sizeY) {
      console.output("placew: coordinates are out of bounds.", RED);
      return false;
   }

   state.map.setWall(x, y, id);
   console.output(TextFormat("placew: set wall at coordinates (X %d; Y %d) to '%s'.", x, y, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_fillw(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 6) {
      console.output("fillw: expected 5 arguments.", RED);
      return false;
   }

   int sx, sy, dx, dy;
   blockid_t id;
   try {
      id = stoi(args[1]);

      if (!isBlockIdValid(id)) {
         console.output("fillw: invalid wall id.", RED);
         return false;
      }
   } catch (...) {
      if (!isBlockNameValid(args[1].c_str())) {
         console.output("fillw: expected first argument to either be a valid wall id or name.", RED);
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
      console.output("fillw: expected second, third, fourth and fifth arguments to be numbers.", RED);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= state.map.sizeX || sy >= state.map.sizeY || dx < 0 || dy < 0 || dx >= state.map.sizeX || dy >= state.map.sizeY) {
      console.output("fillw: coordinates are out of bounds.", RED);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         state.map.setWall(x, y, id);
      }
   }
   console.output(TextFormat("fillw: filled all walls from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getBlockNameFromId(id).c_str()));
   return true;
}

bool c_placeq(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 4) {
      console.output("placeq: expected 3 arguments.", RED);
      return false;
   }

   int x, y;
   liquidid_t id;
   try {
      id = stoi(args[1]);

      if (!isLiquidIdValid(id)) {
         console.output("placeq: invalid liquid id.", RED);
         return false;
      }
   } catch (...) {
      if (!isLiquidNameValid(args[1]) && args[1] != "none") {
         console.output("placeq: expected first argument to either be a valid liquid id or name.", RED);
         return false;
      }
      id = (args[1] == "none" ? 0 : getLiquidIdFromName(args[1]));
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("placeq: expected second and third arguments to be numbers.", RED);
      return false;
   }

   if (x < 0 || y < 0 || x >= state.map.sizeX || y >= state.map.sizeY) {
      console.output("placeq: coordinates are out of bounds.", RED);
      return false;
   }

   state.map.deleteBlock(x, y);
   state.map.setLiquid(x, y, id, id == 0 ? 0 : maxLiquidLayers);

   console.output(TextFormat("placeq: set liquid at coordinates (X %d; Y %d) to '%s'.", x, y, getLiquidNameFromId(id).c_str()));
   return true;
}

bool c_fillq(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 6) {
      console.output("fillq: expected 5 arguments.", RED);
      return false;
   }

   int sx, sy, dx, dy;
   liquidid_t id;
   try {
      id = stoi(args[1]);

      if (!isLiquidIdValid(id)) {
         console.output("fillq: invalid liquid id.", RED);
         return false;
      }
   } catch (...) {
      if (!isLiquidNameValid(args[1]) && args[1] != "none") {
         console.output("fillq: expected first argument to either be a valid liquid id or name.", RED);
         return false;
      }
      id = (args[1] == "none" ? 0 : getLiquidIdFromName(args[1]));
   }

   try {
      sx = stoi(args[2]);
      sy = stoi(args[3]);
      dx = stoi(args[4]);
      dy = stoi(args[5]);
   } catch (...) {
      console.output("fillq: expected second, third, fourth and fifth arguments to be numbers.", RED);
      return false;
   }

   if (sx < 0 || sy < 0 || sx >= state.map.sizeX || sy >= state.map.sizeY || dx < 0 || dy < 0 || dx >= state.map.sizeX || dy >= state.map.sizeY) {
      console.output("fillq: coordinates are out of bounds.", RED);
      return false;
   }

   if (sy > dy) std::swap(sy, dy);
   if (sx > dx) std::swap(sx, dx);

   for (int y = sy; y < dy; ++y) {
      for (int x = sx; x < dx; ++x) {
         state.map.deleteBlock(x, y);
         state.map.setLiquid(x, y, id, id == 0 ? 0 : maxLiquidLayers);
      }
   }
   console.output(TextFormat("fillq: filled all liquids from coordinates (X %d; Y %d) to (X %d; Y %d) as %s.", sx, sy, dx, dy, getLiquidNameFromId(id).c_str()));
   return true;
}

bool c_placef(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 4) {
      console.output("placef: expected 3 arguments.", RED);
      return false;
   }

   int x, y;
   furnitureid_t id;
   try {
      id = stoi(args[1]);

      if (!isFurnitureIdValid(id)) {
         console.output("placef: invalid furniture id.", RED);
         return false;
      }
   } catch (...) {
      if (!isFurnitureNameValid(args[1].c_str())) {
         console.output("placef: expected first argument to either be a valid furniture id or name.", RED);
         return false;
      }
      id = getFurnitureIdFromName(args[1].c_str());
   }

   try {
      x = stoi(args[2]);
      y = stoi(args[3]);
   } catch (...) {
      console.output("placef: expected second and third arguments to be numbers.", RED);
      return false;
   }

   if (x < 0 || y < 0 || x >= state.map.sizeX || y >= state.map.sizeY) {
      console.output("placef: coordinates are out of bounds.", RED);
      return false;
   }

   generateFurniture(x, y, state.map, id, state.player.flipX);
   console.output(TextFormat("placef: attempted to place furniture '%s' at coordinates (X %d; Y %d).", getFurnitureNameFromId(id).c_str(), x, y));
   return true;
}

bool c_give(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 3 && args.size() != 2) {
      console.output("give: expected 1 or 2 arguments.", RED);
      return false;
   }

   Item item;
   item.count = 1;

   try {
      if (isItemNameValid(args[1])) {
         item.id = getItemIdFromName(args[1]);
      }
      else {
         console.output("give: invalid first argument, expected valid item name.", RED);
         return false;
      }

      if (args.size() == 3) {
         item.count = stoi(args[2]);
      }

      if (item.count > getItemData(item.id).stackSize) {
         console.output("give: invalid second argument, count exceeded stack size. Defaulting to stack size.", RED);
         item.count = getItemData(item.id).stackSize;
      }
   } catch (...) {
      console.output("give: expected second argument to be number.", RED);
      return false;
   }

   console.output(TextFormat("give: gave %d of '%s'.", item.count, args[1].c_str()));
   state.inventory.placeItemOrDrop(item);
   return true;
}

bool c_cinv(Console &console, std::vector<std::string> &args, GameState &state) {
   state.inventory.selection = {};
   for (int i = 0; i < inventorySlots; ++i) {
      state.inventory.items[i] = {};
   }
   return true;
}

bool c_set(Console &console, std::vector<std::string> &args, GameState &state) {
   if (args.size() != 3) {
      console.output("set: expected 2 arguments.", RED);
      return false;
   }
   
   auto it = console.vars.find(args[1]);
   if (it == console.vars.end()) {
      console.output(TextFormat("set: unknown variable '%s'.", args[1].c_str()), RED);
      return false;
   }

   // print the real value since we aren't sure if the conversion from string actually succeeded
   switch (it->second.type) {
   case VariableType::boolean:
      *it->second.bvalue = getBoolValue(args[2]);
      console.output(TextFormat("set: set variable '%s' to '%s'.", args[1].c_str(), (*it->second.bvalue ? "true" : "false")));
      break;
   case VariableType::integer:
      *it->second.ivalue = getIntValue(args[2]);
      console.output(TextFormat("set: set variable '%s' to '%d'.", args[1].c_str(), *it->second.ivalue));
      break;
   case VariableType::floating:
      *it->second.fvalue = getFloatValue(args[2]);
      console.output(TextFormat("set: set variable '%s' to '%.2f'.", args[1].c_str(), *it->second.fvalue));
      break;
   case VariableType::string:
      *it->second.svalue = getStringValue(args[2]);
      console.output(TextFormat("set: set variable '%s' to '%s'.", args[1].c_str(), it->second.svalue->c_str()));
      break;
   default: break;
   }
   return true;
}

bool c_list(Console &console, std::vector<std::string> &args, GameState &state) {
   console.output("Variables:", GRAY);
   for (auto &[key, value] : console.vars) {
      std::string msg = key + ": ";
      switch (value.type) {
      case VariableType::boolean:
         msg += (*value.bvalue ? "true" : "false");
         break;
      case VariableType::integer:
         msg += std::to_string(*value.ivalue);
         break;
      case VariableType::floating:
         msg += std::to_string(*value.fvalue);
         break;
      case VariableType::string:
         msg += *value.svalue;
         break;
      default: break;
      }
      console.output(msg);
   }
   console.output("Scroll back with the scroll wheel to see more variables.", GRAY);
   console.output("Changing these might result in crashes or unrecoverable worlds. Use at your own risk.", GRAY);
   return true;
}

// command map

using Command = bool(*)(Console&, std::vector<std::string>&, GameState&);
static inline const std::unordered_map<std::string, Command> commands {
   {"help", c_help}, {"echo", c_echo}, {"tp", c_tp}, {"spawnpoint", c_spawnpoint}, {"pos", c_pos}, {"clear", c_clear},
   {"cinv", c_cinv}, {"exit", c_exit}, {"hp", c_hp}, {"maxhp", c_maxhp}, {"kill", c_kill}, {"time", c_time}, {"hist", c_hist},
   {"chist", c_chist}, {"place", c_place}, {"fill", c_fill}, {"placew", c_placew}, {"fillw", c_fillw}, {"placeq", c_placeq},
   {"fillq", c_fillq}, {"placef", c_placef}, {"give", c_give}, {"set", c_set}, {"list", c_list},
};

// init

void Console::init(GameState &state) {
   input.wrapinput = false;
   input.cursor = true;
   input.textOrigin = CENTER_RIGHT;
   input.init(getFont("andy"), {0}, TOP_LEFT, 512, "'help' for a list of commands.");
   updateResponsiveness();

   // player
   vars["player.position.x"] = createVariable(&state.player.position.x);
   vars["player.position.y"] = createVariable(&state.player.position.y);
   vars["player.deathTimer"] = createVariable(&state.deathTimer);
   vars["player.timeToRespawn"] = createVariable(&state.timeToRespawn);
   vars["player.maxPickupRange"] = createVariable(&state.maxPickupRange);
   vars["player.maxToolRange"] = createVariable(&state.maxToolRange);
   vars["player.spawnPos.x"] = createVariable(&state.player.spawnPos.x);
   vars["player.spawnPos.y"] = createVariable(&state.player.spawnPos.y);
   vars["player.velocity.x"] = createVariable(&state.player.velocity.x);
   vars["player.velocity.y"] = createVariable(&state.player.velocity.y);
   vars["player.previousPosition.x"] = createVariable(&state.player.previousPosition.x);
   vars["player.previousPosition.y"] = createVariable(&state.player.previousPosition.y);
   vars["player.delta.x"] = createVariable(&state.player.delta.x);
   vars["player.delta.y"] = createVariable(&state.player.delta.y);
   vars["player.delta.y"] = createVariable(&state.player.delta.y);
   vars["player.feetCollision"] = createVariable(&state.player.feetCollision);
   vars["player.torsoCollision"] = createVariable(&state.player.torsoCollision);
   vars["player.feetCollisionY"] = createVariable(&state.player.feetCollisionY);
   vars["player.onGround"] = createVariable(&state.player.onGround);
   vars["player.shouldBounce"] = createVariable(&state.player.shouldBounce);
   vars["player.coyoteTimer"] = createVariable(&state.player.coyoteTimer);
   vars["player.foxTimer"] = createVariable(&state.player.foxTimer);
   vars["player.maximumY"] = createVariable(&state.player.maximumY);
   vars["player.waterMultiplier"] = createVariable(&state.player.waterMultiplier);
   vars["player.iceMultiplier"] = createVariable(&state.player.iceMultiplier);
   vars["player.fallTimer"] = createVariable(&state.player.fallTimer);
   vars["player.walkTimer"] = createVariable(&state.player.walkTimer);
   vars["player.jumpTimer"] = createVariable(&state.player.jumpTimer);
   vars["player.walkFrame"] = createVariable(&state.player.walkFrame);
   vars["player.frameX"] = createVariable(&state.player.frameX);
   vars["player.flipX"] = createVariable(&state.player.flipX);
   vars["player.ignoreCollision"] = createVariable(&state.player.ignoreCollision);
   vars["player.breathFrameCounter"] = createVariable(&state.player.breathFrameCounter);
   vars["player.breath"] = createVariable(&state.player.breath);
   vars["player.lastHearts"] = createVariable(&state.player.lastHearts);
   vars["player.hearts"] = createVariable(&state.player.hearts);
   vars["player.maxHearts"] = createVariable(&state.player.maxHearts);
   vars["player.regenerationFrameCounter"] = createVariable(&state.player.regenerationFrameCounter);
   vars["player.immunityFrame"] = createVariable(&state.player.immunityFrame);
   vars["player.timeSinceLastDamage"] = createVariable(&state.player.timeSinceLastDamage);
   vars["player.timeSpentRegenerating"] = createVariable(&state.player.timeSpentRegenerating);
   vars["player.regenSpeedMultiplier"] = createVariable(&state.player.regenSpeedMultiplier);
   vars["player.regeneration"] = createVariable(&state.player.regeneration);
   vars["player.displayHearts"] = createVariable(&state.player.displayHearts);
   vars["player.displayBreath"] = createVariable(&state.player.displayBreath);
   vars["player.placedBlockAnimation"] = createVariable(&state.player.placedBlockAnimation);
   vars["player.canPlaceBlock"] = createVariable(&state.player.canPlaceBlock);
   vars["player.blockPlacementSpeed"] = createVariable(&state.player.blockPlacementSpeed);
   vars["player.blockPlacementTimer"] = createVariable(&state.player.blockPlacementTimer);
   vars["player.breakingFurniture"] = createVariable(&state.player.breakingFurniture);
   vars["player.breakingWall"] = createVariable(&state.player.breakingWall);
   vars["player.breakAnimation"] = createVariable(&state.player.breakAnimation);
   vars["player.lastBreaking.x"] = createVariable(&state.player.lastBreakingX);
   vars["player.lastBreaking.y"] = createVariable(&state.player.lastBreakingY);
   vars["player.breakingBlock"] = createVariable(&state.player.breakingBlock);
   vars["player.breakTime"] = createVariable(&state.player.breakTime);
   vars["player.breakSpeed"] = createVariable(&state.player.breakSpeed);
   vars["player.breakAnimationTimer"] = createVariable(&state.player.breakAnimationTimer);
   vars["player.creative"] = createVariable(&state.player.creative);

   // inventory
   vars["inventory.open"] = createVariable(&state.inventory.open);
   vars["inventory.selected"] = createVariable(&state.inventory.selected);
   vars["inventory.selection.item.count"] = createVariable(&state.inventory.selection.item.count);
   vars["inventory.selection.item.favorite"] = createVariable(&state.inventory.selection.item.favorite);
   vars["inventory.selection.lastSelection"] = createVariable(&state.inventory.selection.lastSelection);
   vars["inventory.selection.selected"] = createVariable(&state.inventory.selection.selected);

   // map
   vars["map.sizeX"] = createVariable(&state.map.sizeX);
   vars["map.sizeY"] = createVariable(&state.map.sizeY);
   vars["map.name"] = createVariable(&state.worldName);

   // physics
   vars["physics.counter"] = createVariable(&state.physicsCounter);
   vars["physics.ticks"] = createVariable(&state.physicsTicks);
   vars["physics.grassGrowSpeed.min"] = createVariable(&state.grassGrowSpeedMin);
   vars["physics.grassGrowSpeed.max"] = createVariable(&state.grassGrowSpeedMax);

   // camera
   vars["camera.offset.x"] = createVariable(&state.camera.offset.x);
   vars["camera.offset.y"] = createVariable(&state.camera.offset.y);
   vars["camera.target.x"] = createVariable(&state.camera.target.x);
   vars["camera.target.y"] = createVariable(&state.camera.target.y);
   vars["camera.rotation"] = createVariable(&state.camera.rotation);
   vars["camera.zoom"] = createVariable(&state.camera.zoom);
   vars["camera.bounds.x"] = createVariable(&state.cameraBounds.x);
   vars["camera.bounds.y"] = createVariable(&state.cameraBounds.y);
   vars["camera.bounds.width"] = createVariable(&state.cameraBounds.width);
   vars["camera.bounds.height"] = createVariable(&state.cameraBounds.height);
   vars["camera.zoom.min"] = createVariable(&state.minCameraZoom);
   vars["camera.zoom.max"] = createVariable(&state.maxCameraZoom);
   vars["camera.followSpeed"] = createVariable(&state.cameraFollowSpeed);
}

void Console::output(const std::string &string, Color color) {
   size_t last = text.size();
   divideTextInPlace(text, string, getFont("andy"), input.rect.width - mapRatioToX(0.01f, WINDOW_AREA, CUBIC_RATIO), getFontSizeScaled(consoleFontSize));

   for (size_t i = last; i < text.size(); ++i) {
      textColors.push_back(color);
   }
}

// Update

void Console::update(float dt, GameState &state) {
   bool wastyping = input.typing;
   input.update(dt);

   if (wastyping && !input.typing && IsKeyPressed(KEY_ENTER)) {
      input.typing = true;
      lex(state);
      input.text.clear();
      scrollback = std::max(0, (int)text.size() - maxLines);
      historyIndex = 0;
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
      float wheel = GetMouseWheelMove();
      if (wheel >= 1.0f) {
         scrollback = std::max(0, scrollback - 1);
      } else if (wheel <= -1.0f) {
         scrollback = std::min(std::max(0, (int)text.size() - maxLines), scrollback + 1);
      }
   }
}

void Console::updateResponsiveness() {
   input.rect = mapRatioToArea(R4(0.0f, 1.0f, 0.92f, 0.05f), BOTTOM_LEFT, WINDOW_AREA, CUBIC_RATIO);
}

// Render

void Console::render() {
   if (!input.typing) return;
   Rectangle area = mapRatioToArea(R4(0.0f, 1.0f, 0.92f, 0.3f), BOTTOM_LEFT, WINDOW_AREA, CUBIC_RATIO);
   Rectangle outputArea = mapRatioToArea(R4(0.0f, 1.0f, 0.92f, 0.3f - 0.05f), BOTTOM_LEFT, WINDOW_AREA, CUBIC_RATIO);

   Font &font = getFont("andy");
   float constX = area.x + mapRatioToX(0.005f, WINDOW_AREA, CUBIC_RATIO);
   float constY = area.y + mapRatioToY(0.005f, WINDOW_AREA, CUBIC_RATIO);
   float paddingY = mapRatioToHeight((1.0f - 0.005f) / maxLines, outputArea, CUBIC_RATIO);

   drawRect(input.rect, TOP_LEFT, Fade(BLACK, 0.9f));
   drawRect(area, TOP_LEFT, Fade(BLACK, 0.75f));
   input.render();

   for (int i = scrollback; i < scrollback + maxLines && (size_t)i < text.size(); ++i) {
      drawText(font, {constX, constY + (i - scrollback) * paddingY}, text[i].c_str(), getFontSizeScaled(consoleFontSize), TOP_LEFT, textColors[i]);
   }
}

// Lexing/command logic

void Console::lex(GameState &state) {
   if (input.text.empty()) {
      return;
   }
   
   // Only push back a command to history if it wasn't reran
   if (history.size() == 0 || input.text != history.back()) {
      history.push_back(input.text);
   }

   std::vector<std::string> args;
   for (size_t index = 0; index < input.text.size(); ++index) {
      char ch = input.text[index];

      if (ch == ';') {
         if (args.empty()) {
            output("operator ';': no command to execute.", RED);
            return;
         }
         handleCommand(args, state);
         args.clear();
      } else if (ch == '&') {
         if (args.empty()) {
            output("operator '&': no command to execute.", RED);
            return;
         }
         
         if (!handleCommand(args, state)) return;
         args.clear();
      } else if (ch == '|') {
         if (args.empty()) {
            output("operator '|': no command to execute.", RED);
            return;
         }

         if (handleCommand(args, state)) return;
         args.clear();
      } else if (ch == '$') {
         std::string var;
         index += 1;
         if (index >= input.text.size()) {
            output("operator '$': no variable present.", RED);
            return;
         }

         for (ch = input.text[index]; index < input.text.size() && (std::isalnum(ch) || ch == '.'); ch = input.text[++index])
            var.push_back(ch);

         auto it = vars.find(var);
         if (it == vars.end()) {
            output(TextFormat("operator '$': no such variable '%s'.", var.c_str()), RED);
            return;
         }

         switch(it->second.type) {
         case VariableType::boolean:
            args.push_back((*it->second.bvalue ? "true" : "false"));
            break;
         case VariableType::integer:
            args.push_back(std::to_string(*it->second.ivalue));
            break;
         case VariableType::floating:
            args.push_back(std::to_string(*it->second.fvalue));
            break;
         case VariableType::string:
            args.push_back(*it->second.svalue);
            break;
         default: break;
         }
         index -= 1;
      } else if (ch == '"') {
         std::string str;
         index += 1;
         if (index >= input.text.size()) {
            output("operator '\"': unterminated string.", RED);
            return;
         }

         for (ch = input.text[index]; index < input.text.size() && ch != '"'; ch = input.text[++index])
            str.push_back(ch);

         if (index >= input.text.size() || ch != '"') {
            output("operator '\"': unterminated string.", RED);
            return;
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
   handleCommand(args, state);
}

bool Console::handleCommand(std::vector<std::string> &args, GameState &state) {
   if (args.empty()) {
      return false;
   }
   
   if (auto it = commands.find(args[0]); it != commands.end()) {
      return it->second(*this, args, state);
   } else {
      output("Invalid command. See 'help' for a list of commands.", RED);
      return false;
   }
}
