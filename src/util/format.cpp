#include "mngr/resource.hpp"
#include "util/format.hpp"

void wrapText(std::string &string, float maxWidth, float fontSize, float spacing) {
   auto wrap = [&string, maxWidth, fontSize, spacing]() -> bool {
      return MeasureTextEx(getFont("andy"), string.c_str(), fontSize, spacing).x > maxWidth;
   };

   if (!wrap()) {
      return;
   }

   const std::string original = string;
   std::string_view split = original; // string cannot be viewed because it gets changed
   std::stringstream result;

   while (wrap()) {
      size_t left = 0, right = split.size();
      std::string_view truncated;

      while (left < right) {
         size_t mid = (left + right) / 2;
         truncated = split.substr(0, mid);
         string = std::string(truncated) + "-";

         if (wrap()) {
            right = mid;
         } else {
            left = mid + 1;
         }
      }
      const bool punctuation = (std::ispunct(split.at(left - 1)));
      truncated = split.substr(0, (punctuation ? left : left - 1));
      split = split.substr((punctuation ? left : left - 1));

      const bool dash = std::isalpha(truncated.back()) && std::isalpha(split.front());
      if (std::isspace(split.front())) {
         split = split.substr(1);
      }

      result << truncated << (dash ? "-\n" : "\n");
      string = std::string(split);

      if (!wrap()) {
         result << split;
         break;
      }
   }
   string = result.str();
}
