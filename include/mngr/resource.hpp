#ifndef MNGR_RESOURCE_HPP
#define MNGR_RESOURCE_HPP

#include <filesystem>
#include <string>
#include <unordered_map>
#include <raylib.h>

// Resource manager

struct ResourceManager {
   ResourceManager() = default;
   ~ResourceManager() = default;

   static ResourceManager& get() {
      static ResourceManager resourceManager;
      return resourceManager;
   }

   // Load functions

   Texture& loadTexture(const std::string& name, const std::filesystem::path& path);
   Font& loadFont(const std::string& name, const std::filesystem::path& path);

   void loadTextures();
   void loadFonts();

   // Get functions

   Texture& getTexture(const std::string& name);
   Font& getFont(const std::string& name);

   // Fallback functions

   Texture& getFallbackTexture();
   Font& getFallbackFont();

private:
   std::unordered_map<std::string, Texture> textures;
   std::unordered_map<std::string, Font> fonts;
};

#endif
