#ifndef MNGR_RESOURCE_HPP
#define MNGR_RESOURCE_HPP

#include <raylib.h>
#include <filesystem>
#include <string>

// Load functions

Texture& loadTexture(const std::string& name, const std::filesystem::path& path);
Font& loadFont(const std::string& name, const std::filesystem::path& path);
void loadTextures();
void loadFonts();

// Get functions

Texture& getTexture(const std::string& name);
Font& getFont(const std::string& name);

#endif
