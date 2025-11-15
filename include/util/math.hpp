#ifndef UTIL_MATH_HPP
#define UTIL_MATH_HPP

// Math functions

template<typename T>
constexpr T min(T a, T b) {
   return (a > b ? b : a);
}

template<typename T>
constexpr T max(T a, T b) {
   return (a > b ? a : b);
}

template<typename T>
constexpr T clamp(T v, T lo, T hi) {
   return (v < lo ? lo : (v > hi ? hi : v));
}

template<typename T>
constexpr T abs(T a) {
   return (a < 0 ? -a : a);
}

constexpr float lerp(float a, float b, float t) {
   return a + (b - a) * t;
}

constexpr bool floatEquals(float a, float b) {
   return abs(a - b) <= .001f;
}

constexpr bool floatIsZero(float a) {
   return floatEquals(a, 0.f);
}

#endif
