#include "mngr/resource.hpp"
#include "util/text.hpp"

void wrapText(std::string& string, float maxWidth, float fontSize, float spacing) {
   auto& font = getFont("andy");

   if (MeasureTextEx(font, string.c_str(), fontSize, spacing).x <= maxWidth) {
      return;
   }

   auto original = string;
   std::string_view view = original, split = original;
   std::stringstream result;

   while (MeasureTextEx(font, string.c_str(), fontSize, spacing).x > maxWidth) {
      size_t left = 0, right = split.size();
      std::string_view truncated;

      while (left < right) {
         size_t mid = (left + right) / 2;
         truncated = split.substr(0, mid);
         string = std::string(truncated) + "-";

         if (MeasureTextEx(font, string.c_str(), fontSize, spacing).x > maxWidth) {
            right = mid;
         } else {
            left = mid + 1;
         }
      }
      truncated = split.substr(0, left - 1);
      split = split.substr(left - 1);

      bool dash = isalpha(truncated.back()) and isalpha(split.front());
      result << truncated << (dash ? "-\n" : "\n");
      string = std::string(split);

      if (MeasureTextEx(font, string.c_str(), fontSize, spacing).x <= maxWidth) {
         result << split;
         break;
      }
   }
   string = result.str();
}
