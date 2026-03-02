#include "objs/console.hpp"
#include "mngr/resource.hpp"
#include "objs/inventory.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <sstream>

// Constants

constexpr int maxVisibleLinesOnScreenAtOnce = 6;

// init

void Console::init() {
   input.rectangle = {500.0f, GetScreenHeight() - 25.0f, 1000.0f, 50.0f};
   input.fallback = "'help' for a list of commands.";
   input.wrapinput = false;
   input.maxChars = 512;
}

void Console::divideOutput(const std::string &string) {
   divideText(output, string, input.rectangle.width - 10.0f, 35, 1.0f);
}

void Console::update(Map &map, Player &player, Inventory &inventory) {
   bool wastyping = input.typing;
   outputDelay -= GetFrameTime();
   input.update();

   if (wastyping && !input.typing && IsKeyPressed(KEY_ENTER)) {
      handleCommand(map, player, inventory);
      input.typing = true;
   }

   bool shouldRenderBefore = shouldRender;
   shouldRender = outputDelay > 0.0f;

   if (shouldRenderBefore && !shouldRender) {
      fadingout = true;
      fadeoutTimer = 0.5f;
   }

   if (fadingout) {
      fadeoutTimer -= GetFrameTime();
      input.alpha = fadeoutTimer * 2.0f;;
      fadingout = (fadeoutTimer > 0.0f);
   } else {
      input.alpha = 1.0f;
   }
   renderInGameState = shouldRender || fadingout || input.typing;

   if (renderInGameState) {
      float thing = GetMouseWheelMove();
      if (thing >= 1.0f) {
         scrollback = std::max(0, scrollback - 1);
      } else if (thing <= -1.0f) {
         scrollback = std::min((int)output.size() - maxVisibleLinesOnScreenAtOnce, scrollback + 1);
      }
   }
}

void Console::render() {
   drawRect(input.rectangle, Fade(BLACK, (fadingout ? fadeoutTimer * 1.8f : 0.9f)));
   input.render();
   drawRect({input.rectangle.x, input.rectangle.y - 125.0f - input.rectangle.height, input.rectangle.width, input.rectangle.height + 250.0f}, Fade(BLACK, (fadingout ? fadeoutTimer * 1.5f : 0.75f)));

   for (int i = scrollback; i < scrollback + maxVisibleLinesOnScreenAtOnce && (size_t)i < output.size(); ++i) {
      DrawTextPro(getFont("andy"), output[i].c_str(), {input.rectangle.x - input.rectangle.width / 2.0f + 5.0f, (input.rectangle.y - 125.0f) - (input.rectangle.height + 250.0f) / 2.0f + (i - scrollback) * 40}, {0, getOrigin(output[i].c_str(), 35, 1).y}, 0, 35, 1, Fade(WHITE, (fadingout ? fadeoutTimer * 2.0f : 1.0f)));
   }
}

// Commands

void Console::handleCommand(Map &map, Player &player, Inventory &inventory) {
   std::stringstream s (input.text);
   std::string temp;
   Args args;

   while (s >> temp) {
      args.push_back(temp);
   }

   if (args.empty()) {
      return;
   }

   if (args[0] == "help") {
      help(args, map, player, inventory);
   } else if (args[0] == "tp") {
      tp(args, map, player, inventory);
   } else {
      divideOutput("Invalid command. See 'help' for a list of commands.");
   }

   input.text.clear();
   outputDelay = 10.0f;
   scrollback = std::max(0, (int)output.size() - maxVisibleLinesOnScreenAtOnce);
}

void Console::help(const Args&, Map&, Player&, Inventory&) {
   divideOutput("tp X Y - teleport player to the given coordinates.");
}

void Console::tp(const Args &args, Map &map, Player &player, Inventory&) {
   if (args.size() != 3) {
      divideOutput("tp: expected exactly 2 arguments.");
      return;
   }
   int x, y;

   // fuck this function
   try {
      x = stoi(args[1]);
      y = stoi(args[2]);
   } catch (...) {
      divideOutput("tp: expected both arguments to be numbers.");
      return;
   }

   if (x < 0 || y < 0 || x >= map.sizeX || y >= map.sizeY) {
      divideOutput("tp: coordinates are out of bounds.");
      return;
   }

   player.position.x = x;
   player.position.y = y;
   divideOutput(TextFormat("tp: teleported to (X %d; Y %d).", x, y));
}
