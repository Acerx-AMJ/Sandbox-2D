#ifndef UTIL_ASSERT_HPP
#define UTIL_ASSERT_HPP

#include <iostream>
#include <sstream>

// String conversion functions

template<typename T>
std::string toString(const T& value) {
   std::stringstream s;
   s << std::boolalpha << value;
   return s.str();
}

// Format functions

template<typename... Args>
std::string format(const char* base, const Args&... args) {
   std::string result = base;

   size_t pos = 0;
   (( pos = result.find("{}", pos),
      pos != std::string::npos
         ? result = result.replace(pos, 2, toString(args))
         : result
   ), ...);
   return result;
}

// Debug print functions

template<typename... Args>
void printf(const char* base, const Args&... args) {
   std::cout << format(base, args...) << '\n';
}

template<typename... Args>
void print(const Args&... args) {
   ((std::cout << std::boolalpha << args << ' '), ...);
   std::cout << '\n';
}

// Macros

#undef warn
#undef assert
#undef debugPrint

#define warn(base, ...) std::cout << "WARNING: '" << __FILE__ << ":" << __LINE__ << "': " << format(base, __VA_ARGS__) << '\n';
#define assert(condition, base, ...) if (not (condition)) { std::cout << "ERROR: '" << __FILE__ << ":" << __LINE__ << "': " << format(base, __VA_ARGS__) << '\n'; std::exit(-1); }
#define debugPrint() std::cout << "LINE: " << __LINE__ << '\n';

#endif
