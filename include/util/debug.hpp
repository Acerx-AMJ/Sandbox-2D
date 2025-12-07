#ifndef UTIL_DEBUG_HPP
#define UTIL_DEBUG_HPP

constexpr bool isProjectInDebug() {
#ifndef NDEBUG
   return true;
#else
   return false;
#endif
}

bool isDebugModeActive();
bool isStatModeActive();

void updateDebugOverlay();
void renderDebugOverlay();

#endif
