#ifndef UTIL_STRARRAY_HPP
#define UTIL_STRARRAY_HPP

#include <unordered_map>

template<typename T>
struct StrArray {
   StrArray(std::initializer_list<T> list) {
      size_t i = 0;
      for (T el : list)
         map[el] = i++;
   }

   inline size_t &operator [] (const T &key) {
      return map[key];
   }

   inline size_t at(const T &key) const {
      return map.at(key);
   }

   // Members

   std::unordered_map<T, size_t> map;
};

#endif
