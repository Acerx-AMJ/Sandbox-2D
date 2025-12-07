#ifndef MNGR_SOUND_HPP
#define MNGR_SOUND_HPP

#include <raylib.h>
#include <string>
#include <vector>

// Load functions

void loadSound(const std::string &name, const std::string &path);
void loadMusic(const std::string &name, const std::string &path);
void saveSound(const std::string &name, const std::vector<std::string> &soundList);
void loadSounds();
void loadMusic();

// Play functions

void playSound(const std::string &name, float volume = 1.f);
void playMusic(const std::string &name);

// Get functions

Sound& getSound(const std::string &name);
Music& getMusic(const std::string &name);

// Update functions

void updateMusic();

#endif
