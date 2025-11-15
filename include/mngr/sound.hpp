#ifndef MNGR_SOUND_HPP
#define MNGR_SOUND_HPP

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <raylib.h>

// Sound manager

struct SoundManager {
   SoundManager() = default;
   ~SoundManager() = default;

   static SoundManager& get() {
      static SoundManager soundManager;
      return soundManager;
   }

   // Load functions

   void loadSound(const std::string& name, const std::filesystem::path& path);
   void loadMusic(const std::string& name, const std::filesystem::path& path);
   void saveSound(const std::string& name, const std::vector<std::string>& names);

   void loadSounds();
   void loadMusic();

   // Play functions

   void play(const std::string& name);
   void playMusic(const std::string& name);

   // Get functions

   Sound& getSound(const std::string& name);
   Music& getMusic(const std::string& name);

   bool soundExists(const std::string& name);
   bool musicExists(const std::string& name);

   // Update functions

   void update();

private:
   std::unordered_map<std::string, std::vector<Sound*>> savedSounds;
   std::unordered_map<std::string, Sound> sounds;
   std::unordered_map<std::string, Music> music;
   Music* currentMusic = nullptr;
};

#endif
