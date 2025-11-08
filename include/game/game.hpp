#ifndef GAME_GAME_HPP
#define GAME_GAME_HPP

// Includes

#include "game/state.hpp"

// Game class

class Game {
   States states;

public:
   Game();
   ~Game();

   void run();
};

#endif
