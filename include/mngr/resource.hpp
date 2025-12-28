#ifndef MNGR_RESOURCE_HPP
#define MNGR_RESOURCE_HPP

#include <raylib.h>
#include <string>

// Load functions

Texture& loadTexture(const std::string &name, const std::string &path);
Font& loadFont(const std::string &name, const std::string &path);
Shader& loadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath);

void loadTextures();
void loadFonts();
void loadShaders();

// Get functions

Texture& getTexture(const std::string &name);
Font& getFont(const std::string &name);
Shader& getShader(const std::string &name);

#endif
