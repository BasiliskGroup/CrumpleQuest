#include "audio_system.h"
#include "audio_group.h"
#include "sound.h"
#include <stdexcept>

namespace audio {

AudioSystem::AudioSystem() : master_volume_(1.0f) {
  if (ma_engine_init(nullptr, &engine_) != MA_SUCCESS) {
    throw std::runtime_error("Failed to initialize audio engine");
  }
}

AudioSystem::~AudioSystem() {
  groups_.clear();  // Destroy all groups first, while engine is still valid
  
  // Stop the engine to flush all remaining sounds
  ma_engine_stop(&engine_);
  
  // Then uninit the engine
  ma_engine_uninit(&engine_);
}

Sound* AudioSystem::CreateSound(const std::string& filepath, const std::string& group_name) {
  AudioGroup* group = nullptr;
  if (!group_name.empty()) {
    auto it = groups_.find(group_name);
    if (it != groups_.end()) {
      group = it->second.get();
    }
  }
  auto sound = Sound::Create(&engine_, filepath, group);
  return sound.release();
}

AudioGroup* AudioSystem::CreateGroup(const std::string& name) {
  auto group = AudioGroup::Create(&engine_);
  AudioGroup* group_ptr = group.get();
  groups_[name] = std::move(group);
  return group_ptr;
}

void AudioSystem::SetMasterVolume(float volume) {
  master_volume_ = volume;
  ma_engine_set_volume(&engine_, master_volume_);
}

}  // namespace audio