#ifndef UTIL_RANDOM_HPP
#define UTIL_RANDOM_HPP

#include <cstdlib>
#include <vector>

inline int random(int min, int max) {
   return min + (rand() % (max - min + 1));
}

inline float random(float min, float max) {
   return min + (float)rand() / float(RAND_MAX / (max - min));
}

template<class T>
inline T& random(std::vector<T> &vector) {
   return vector[random(0, vector.size() - 1)];
}

inline bool chance(int percent) {
   return random(0, 100) <= percent;
}

#endif
