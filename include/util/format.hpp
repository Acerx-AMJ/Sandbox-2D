#ifndef UTIL_FORMAT_HPP
#define UTIL_FORMAT_HPP

#include <iostream>
#include <sstream>
#include <vector>
#undef assert

// String conversion functions

template<typename T>
inline std::string toString(const T &value) {
   std::stringstream s;
   s << std::boolalpha << value;
   return s.str();
}

// Format functions

template<typename... Args>
std::string format(const char *base, const Args&...args) {
   std::string result = base;

   size_t pos = 0;
   (( pos = result.find("{}", pos),
      pos != std::string::npos
         ? result = result.replace(pos, 2, toString(args))
         : result
   ), ...);
   return result;
}

template<typename... Args>
void warn(const char *base, const Args&...args) {
   std::cout << "WARNING: " << format(base, args...) << "\n";
}

template<typename... Args>
void assert(bool condition, const char *base, const Args&...args) {
   if (!condition) {
      std::cout << "ERROR: " << format(base, args...) << '\n';
      std::exit(-1);
   }
}

#ifndef NDEBUG
#define assertDebug(condition, base, ...) assert(condition, base, __VA_ARGS__)
#else
#define assertDebug(...)
#endif

// Wrap function

void wrapText(std::string &string, float maxWidth, float fontSize, float spacing);
void divideText(std::vector<std::string> &output, const std::string &string, float maxWidth, float fontSize, float spacing);

#endif
