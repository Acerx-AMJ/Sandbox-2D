#include "mngr/sound.hpp"
#include "util/format.hpp" // IWYU pragma: export
#include "util/random.hpp"

// Load functions

void SoundManager::loadSound(const std::string& name, const std::filesystem::path& path) {
   auto newSound = LoadSound(path.c_str());
   sounds[name] = newSound;
}

void SoundManager::loadMusic(const std::string& name, const std::filesystem::path& path) {
   auto newMusic = LoadMusicStream(path.c_str());
   music[name] = newMusic;
}

void SoundManager::saveSound(const std::string& name, const std::vector<std::string>& sounds) {
   std::vector<Sound*> saved;
   for (const auto& identifier: sounds) {
      assert(this->sounds.count(identifier), "Sound '{}' does not exist.", identifier);
      saved.push_back(&this->sounds[identifier]);
   }
   savedSounds[name] = saved;
}

void SoundManager::loadSounds() {
   for (const auto& file: std::filesystem::recursive_directory_iterator("assets/sounds/")) {
      loadSound(file.path().stem().string(), file.path().string());
   }
}

void SoundManager::loadMusic() {
   for (const auto& file: std::filesystem::recursive_directory_iterator("assets/music/")) {
      loadMusic(file.path().stem().string(), file.path().string());
   }
}

// Play functions

void SoundManager::play(const std::string& name) {
   assert(soundExists(name), "Sound '{}' does not exist.", name);
   Sound* sound = nullptr;
   
   if (savedSounds.count(name)) {
      sound = random(savedSounds[name]);
   } else {
      sound = &sounds[name];
   }
   SetSoundPitch(*sound, random(.75f, 1.25f));
   PlaySound(*sound);
}

void SoundManager::playMusic(const std::string& name) {
   assert(musicExists(name), "Music '{}' does not exist.", name);
   currentMusic = &music[name];
}

// Get functions

Sound& SoundManager::getSound(const std::string& name) {
   assert(soundExists(name), "Sound '{}' does not exist.", name);
   return sounds[name];
}

Music& SoundManager::getMusic(const std::string& name) {
   assert(musicExists(name), "Music '{}' does not exist.", name);
   return music[name];
}

bool SoundManager::soundExists(const std::string& name) {
   return savedSounds.count(name) or sounds.count(name);
}

bool SoundManager::musicExists(const std::string& name) {
   return music.count(name);
}

// Update functions

void SoundManager::update() {
   if (currentMusic) {
      UpdateMusicStream(*currentMusic);
   }
}
