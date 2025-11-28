#include "mngr/sound.hpp"
#include "util/format.hpp" // IWYU pragma: export
#include "util/random.hpp"
#include <unordered_map>

// Globals

static std::unordered_map<std::string, std::vector<Sound*>> savedSounds;
static std::unordered_map<std::string, Sound> sounds;
static std::unordered_map<std::string, Music> music;
static Music* currentMusic = nullptr;

// Load functions

void loadSound(const std::string& name, const std::filesystem::path& path) {
   auto newSound = LoadSound(path.string().c_str());
   sounds[name] = newSound;
}

void loadMusic(const std::string& name, const std::filesystem::path& path) {
   auto newMusic = LoadMusicStream(path.string().c_str());
   music[name] = newMusic;
}

void saveSound(const std::string& name, const std::vector<std::string>& sounds) {
   std::vector<Sound*> saved;
   for (const auto& identifier: sounds) {
      assert(::sounds.count(identifier), "Sound '{}' does not exist.", identifier);
      saved.push_back(&::sounds[identifier]);
   }
   savedSounds[name] = saved;
}

void loadSounds() {
   std::filesystem::create_directories("assets/sounds/");
   for (const auto& file: std::filesystem::recursive_directory_iterator("assets/sounds/")) {
      loadSound(file.path().stem().string(), file.path().string());
   }
}

void loadMusic() {
   std::filesystem::create_directories("assets/music/");
   for (const auto& file: std::filesystem::recursive_directory_iterator("assets/music/")) {
      loadMusic(file.path().stem().string(), file.path().string());
   }
}

// Play functions

void playSound(const std::string& name) {
   assert(savedSounds.count(name) or sounds.count(name), "Sound '{}' does not exist.", name);
   Sound* sound = nullptr;
   
   if (savedSounds.count(name)) {
      sound = savedSounds[name][random(0, savedSounds[name].size() - 1)];
   } else {
      sound = &sounds[name];
   }
   SetSoundPitch(*sound, random(.75f, 1.25f));
   PlaySound(*sound);
}

void playMusic(const std::string& name) {
   assert(music.count(name), "Music '{}' does not exist.", name);
   currentMusic = &music[name];
}

// Get functions

Sound& getSound(const std::string& name) {
   assert(savedSounds.count(name) or sounds.count(name), "Sound '{}' does not exist.", name);
   return sounds[name];
}

Music& getMusic(const std::string& name) {
   assert(music.count(name), "Music '{}' does not exist.", name);
   return music[name];
}

// Update functions

void updateMusic() {
   if (currentMusic) {
      UpdateMusicStream(*currentMusic);
   }
}
