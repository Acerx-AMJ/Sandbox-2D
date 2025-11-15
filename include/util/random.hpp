#ifndef UTIL_RANDOM_HPP
#define UTIL_RANDOM_HPP

#include <cstdlib>
#include <vector>

// Random functions

inline int random(int min, int max) {
   return min + (rand() % (max - min + 1));
}

inline float random(float min, float max) {
   return min + (float)rand() / (float)RAND_MAX / (max - min);
}

template<typename T>
T& random(const std::vector<T>& vector) {
   return vector[rand() % vector.size()];
}

template<typename T>
T* random(const std::vector<T*>& vector) {
   return vector[rand() % vector.size()];
}

// Chance functions

inline bool chance(int percent) {
   return random(0, 100) <= percent;
}

#endif
