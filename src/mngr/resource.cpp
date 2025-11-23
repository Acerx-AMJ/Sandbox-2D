#include "mngr/resource.hpp"
#include "util/format.hpp"

// Load functions

Texture& ResourceManager::loadTexture(const std::string& name, const std::filesystem::path& path) {
   if (textures.count(name)) {
      return textures[name];
   }

   Texture texture = LoadTexture(path.c_str());
   if (texture.id == 0) {
      warn("Failed to load texture from file '{}'.", path.string());
      return getFallbackTexture();
   }
   textures.insert({name, texture});
   return textures[name];
}

Font& ResourceManager::loadFont(const std::string& name, const std::filesystem::path& path) {
   if (fonts.count(name)) {
      return fonts[name];
   }

   Font font = LoadFontEx(path.c_str(), 120, nullptr, 0);
   SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
   if (font.texture.id == 0) {
      warn("Failed to load font from file '{}'.", path.string());
      return getFallbackFont();
   }
   fonts.insert({name, font});
   return fonts[name];
}

void ResourceManager::loadTextures() {
   for (const auto& file: std::filesystem::recursive_directory_iterator("assets/sprites/")) {
      loadTexture(file.path().stem().string(), file.path());
   }
}

void ResourceManager::loadFonts() {
   for (const auto& file: std::filesystem::recursive_directory_iterator("assets/fonts/")) {
      loadFont(file.path().stem().string(), file.path());
   }
}

// Get functions

Texture& ResourceManager::getTexture(const std::string& name) {
   if (not textures.count(name)) {
      warn("Texture '{}' does not exist, using fallback texture.", name);
      return getFallbackTexture();
   }
   return textures[name];
}

Font& ResourceManager::getFont(const std::string& name) {
   if (not fonts.count(name)) {
      warn("Font '{}' does not exist, using fallback font.", name);
      return getFallbackFont();
   }
   return fonts[name];
}

// Fallback functions

Texture& ResourceManager::getFallbackTexture() {
   static Texture fallbackTexture;
   static bool loaded = false;

   if (not loaded) {
      Image image = GenImageChecked(32, 32, 4, 4, MAGENTA, BLACK);
      fallbackTexture = LoadTextureFromImage(image);
      UnloadImage(image);
      loaded = true;
   }
   return fallbackTexture;
}

Font& ResourceManager::getFallbackFont() {
   static Font fallbackFont = GetFontDefault();
   return fallbackFont;
}
