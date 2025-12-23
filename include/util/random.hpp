#ifndef UTIL_RANDOM_HPP
#define UTIL_RANDOM_HPP

#include <cstdlib>
#include <vector>

// Random functions

inline int random(int min, int max) {
   return min + (rand() % (max - min + 1));
}

inline float random(float min, float max) {
   return min + (float)rand() / float(RAND_MAX / (max - min));
}

inline bool chance(int percent) {
   return random(0, 100) <= percent;
}

// Random vector access functions

template<class T>
inline T& random(std::vector<T> &vector) {
   return vector[random(0, vector.size() - 1)];
}

template<class T>
inline const T& random(const std::vector<T> &vector) {
   return vector.at(random(0, vector.size() - 1));
}

template<class T>
inline T& random(std::vector<T> &&vector) {
   return vector[random(0, vector.size() - 1)];
}

#endif
