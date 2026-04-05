#include "mngr/resource.hpp"
#include "util/format.hpp"

void wrapText(std::string &string, float maxWidth, float fontSize, float spacing) {
   auto wrap = [maxWidth, fontSize, spacing](const std::string &s) -> bool {
      return MeasureTextEx(getFont("andy"), s.c_str(), fontSize, spacing).x > maxWidth;
   };

   if (!wrap(string)) {
      return;
   }

   const std::string original = string;
   std::string_view split = original; // string cannot be viewed because it gets changed
   std::stringstream result;

   while (!split.empty()) {
      std::string current (split);
      if (!wrap(current)) {
         result << split;
         break;
      }

      size_t left = 0, right = split.size();
      while (left < right) {
         size_t mid = (left + right) / 2;
         std::string temp = std::string(split.substr(0, mid)) + "-";

         if (wrap(temp)) {
            right = mid;
         } else {
            left = mid + 1;
         }
      }

      size_t cut = left > 0 ? left - 1 : 0;
      const bool punctuation = (cut < split.size() && std::ispunct(split[cut]));
      if (punctuation) cut += 1;

      std::string_view truncated = split.substr(0, cut);
      std::string_view remainder = split.substr(cut);

      if (!remainder.empty() && std::isspace(remainder.front())) {
         remainder = remainder.substr(1);
      }

      const bool dash = !truncated.empty() && !remainder.empty() && std::isalpha(truncated.back()) && std::isalpha(split.front());
      result << truncated << (dash ? "-\n" : "\n");
      split = remainder;
   }
   string = result.str();
}

void divideText(std::vector<std::string> &output, const std::string &string, float maxWidth, float fontSize, float spacing) {
   std::string view (string), modstr (string); // Hope compiler optimizes this
   
   auto wrap = [&modstr, maxWidth, fontSize, spacing]() -> bool {
      return MeasureTextEx(getFont("andy"), modstr.c_str(), fontSize, spacing).x > maxWidth;
   };

   if (!wrap()) {
      output.push_back(string);
      return;
   }

   while (wrap()) {
      size_t left = 0, right = view.size();
      std::string truncated;

      while (left < right) {
         size_t mid = (left + right) / 2;
         truncated = view.substr(0, mid);
         modstr = std::string(truncated) + "-";

         if (wrap()) {
            right = mid;
         } else {
            left = mid + 1;
         }
      }
      const bool punctuation = (std::ispunct(view.at(left - 1)));
      truncated = view.substr(0, (punctuation ? left : left - 1));
      view = view.substr((punctuation ? left : left - 1));

      const bool dash = std::isalpha(truncated.back()) && std::isalpha(view.front());
      if (std::isspace(view.front())) {
         view = view.substr(1);
      }

      output.push_back(truncated + (dash ? "-\n" : "\n"));
      modstr = view;

      if (!wrap()) {
         output.push_back(view);
         break;
      }
   }
}
