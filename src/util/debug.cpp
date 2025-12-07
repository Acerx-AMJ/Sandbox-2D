#include "util/debug.hpp"
#include <raylib.h>

// Debug globals

static bool debugMode = false;
static bool statMode = false;

// Debug functions

bool isDebugModeActive() {
   return debugMode;
}

bool isStatModeActive() {
   return statMode;
}

// Update functions

void updateDebugOverlay() {
   if (IsKeyReleased(KEY_F3) && isProjectInDebug()) {
      debugMode = !debugMode;
   }

   if (IsKeyReleased(KEY_F2)) {
      statMode = !statMode;
   }
}

void renderDebugOverlay() {
   if (statMode) {
      DrawFPS(10, 10);
   }
}
