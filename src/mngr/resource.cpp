#include "mngr/resource.hpp"
#include "util/format.hpp"
#include <filesystem>
#include <unordered_map>

// Globals

static std::unordered_map<std::string, Texture> textures;
static std::unordered_map<std::string, Font> fonts;
static std::unordered_map<std::string, Shader> shaders;

// Fallback functions

Texture& getFallbackTexture() {
   static Texture fallbackTexture;
   static bool loaded = false;

   if (!loaded) {
      Image image = GenImageChecked(8, 8, 2, 2, MAGENTA, BLACK);
      fallbackTexture = LoadTextureFromImage(image);
      UnloadImage(image);
      loaded = true;
   }
   return fallbackTexture;
}

Font& getFallbackFont() {
   static Font fallbackFont = GetFontDefault();
   return fallbackFont;
}

// Load functions

Texture& loadTexture(const std::string &name, const std::string &path) {
   if (textures.count(name)) {
      return textures[name];
   }

   Texture texture = LoadTexture(path.c_str());
   if (texture.id == 0) {
      warn("Failed to load texture from file '{}'.", path);
      return getFallbackTexture();
   }
   textures.insert({name, texture});
   return textures[name];
}

Font& loadFont(const std::string &name, const std::string &path) {
   if (fonts.count(name)) {
      return fonts[name];
   }

   Font font = LoadFontEx(path.c_str(), 120, nullptr, 0);
   SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

   if (font.texture.id == 0) {
      warn("Failed to load font from file '{}'.", path);
      return getFallbackFont();
   }
   fonts.insert({name, font});
   return fonts[name];
}

Shader& loadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath) {
   if (shaders.count(name)) {
      return shaders[name];
   }

   Shader shader = LoadShader(vertexPath.c_str(), fragmentPath.c_str());
   assert(shader.id != 0, "Failed to load shader from files: vertex: '{}', fragment: '{}'.", vertexPath, fragmentPath);

   shaders.insert({name, shader});
   return shaders[name];
}

void loadTextures() {
   std::filesystem::create_directories("assets/sprites/");
   for (const auto &file: std::filesystem::recursive_directory_iterator("assets/sprites/")) {
      if (file.is_regular_file()) {
         loadTexture(file.path().stem().string(), file.path().string());
      }
   }
}

void loadFonts() {
   std::filesystem::create_directories("assets/fonts/");
   for (const auto &file: std::filesystem::recursive_directory_iterator("assets/fonts/")) {
      if (file.is_regular_file()) {
         loadFont(file.path().stem().string(), file.path().string());
      }
   }
}

void loadShaders() {
   std::filesystem::create_directories("assets/shaders/");
   for (const auto &file: std::filesystem::recursive_directory_iterator("assets/shaders/")) {
      std::string filename = file.path().stem().string();

      if (shaders.count(filename)) {
         continue;
      }

      if (file.path().extension().string() == ".fs") {
         std::filesystem::path vertexPath = format("assets/shaders/{}.vs", filename);
         loadShader(filename, (std::filesystem::exists(vertexPath) ? vertexPath.string() : std::string()), file.path().string());
      } else if (file.path().extension().string() == ".vs") {
         std::filesystem::path fragmentPath = format("assets/shaders/{}.fs", filename);
         loadShader(filename, file.path().string(), (std::filesystem::exists(fragmentPath) ? fragmentPath.string() : std::string()));
      }
   }
}

// Get functions

Texture& getTexture(const std::string &name) {
   if (!textures.count(name)) {
      warn("Texture '{}' does not exist, using fallback texture.", name);
      return getFallbackTexture();
   }
   return textures[name];
}

Font& getFont(const std::string &name) {
   if (!fonts.count(name)) {
      warn("Font '{}' does not exist, using fallback font.", name);
      return getFallbackFont();
   }
   return fonts[name];
}

Shader& getShader(const std::string &name) {
   assert(shaders.count(name), "Shader '{}' does not exist.", name);
   return shaders[name];
}
