#ifndef MNGR_SOUND_HPP
#define MNGR_SOUND_HPP

#include <raylib.h>
#include <filesystem>
#include <string>
#include <vector>

// Load functions

void loadSound(const std::string& name, const std::filesystem::path& path);
void loadMusic(const std::string& name, const std::filesystem::path& path);
void saveSound(const std::string& name, const std::vector<std::string>& names);
void loadSounds();
void loadMusic();

// Play functions

void playSound(const std::string& name);
void playMusic(const std::string& name);

// Get functions

Sound& getSound(const std::string& name);
Music& getMusic(const std::string& name);

// Update functions

void updateMusic();

#endif
