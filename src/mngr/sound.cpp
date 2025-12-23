#include "mngr/sound.hpp"
#include "util/format.hpp" // IWYU pragma: export
#include "util/random.hpp"
#include <filesystem>
#include <unordered_map>

// Constants

constexpr float soundPitchMin = 0.95f;
constexpr float soundPitchMax = 1.05f;

// Globals

static std::unordered_map<std::string, std::vector<std::string>> savedSounds;
static std::unordered_map<std::string, Sound> sounds;
static std::unordered_map<std::string, Music> music;
static Music *currentMusic = nullptr;

// Load functions

void loadSound(const std::string &name, const std::string &path) {
   Sound newSound = LoadSound(path.c_str());
   sounds[name] = newSound;
}

void loadMusic(const std::string &name, const std::string &path) {
   Music newMusic = LoadMusicStream(path.c_str());
   music[name] = newMusic;
}

void saveSound(const std::string &name, const std::vector<std::string> &soundList) {
   savedSounds[name] = soundList;
}

void loadSavedSounds() {
   saveSound("click", {"click1", "click2", "click3"});
   saveSound("hover", {"hover1", "hover2"});
   saveSound("trash", {"trash1", "trash2", "trash3"});
   saveSound("jump", {"jump1", "jump2", "jump3", "jump4"});
   saveSound("footstep", {"footstep1", "footstep2", "footstep3", "footstep4"});
   saveSound("pickup", {"pickup1", "pickup2", "pickup3", "pickup4"});
}

void loadSounds() {
   std::filesystem::create_directories("assets/sounds/");
   for (const auto &file: std::filesystem::recursive_directory_iterator("assets/sounds/")) {
      if (file.is_regular_file()) {
         loadSound(file.path().stem().string(), file.path().string());
      }
   }
}

void loadMusic() {
   std::filesystem::create_directories("assets/music/");
   for (const auto &file: std::filesystem::recursive_directory_iterator("assets/music/")) {
      if (file.is_regular_file()) {
         loadMusic(file.path().stem().string(), file.path().string());
      }
   }
}

// Play functions

void playSound(const std::string &name, float volume) {
   assert(savedSounds.count(name) || sounds.count(name), "Sound '{}' does not exist.", name);
   Sound sound;

   if (savedSounds.count(name)) {
      std::string &randomName = random(savedSounds[name]);
      sound = sounds[randomName];
   } else {
      sound = sounds[name];
   }
   SetSoundPitch(sound, random(soundPitchMin, soundPitchMax));
   SetSoundVolume(sound, volume);
   PlaySound(sound);
}

void playMusic(const std::string &name) {
   assert(music.count(name), "Music '{}' does not exist.", name);
   currentMusic = &music[name];
}

// Get functions

Sound& getSound(const std::string &name) {
   assert(savedSounds.count(name) || sounds.count(name), "Sound '{}' does not exist.", name);
   return sounds[name];
}

Music& getMusic(const std::string &name) {
   assert(music.count(name), "Music '{}' does not exist.", name);
   return music[name];
}

// Update functions

void updateMusic() {
   if (currentMusic) {
      UpdateMusicStream(*currentMusic);
   }
}
