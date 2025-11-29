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
  // Stop the engine to flush all remaining sounds
  ma_engine_stop(&engine_);
  
  // Then uninit the engine
  ma_engine_uninit(&engine_);
}

std::unique_ptr<Sound> AudioSystem::CreateSound(const std::string& filepath, AudioGroup* group) {
  return Sound::Create(&engine_, filepath, group);
}

std::unique_ptr<AudioGroup> AudioSystem::CreateGroup(const std::string& name) {
  return AudioGroup::Create(&engine_);
}

void AudioSystem::SetMasterVolume(float volume) {
  master_volume_ = volume;
  ma_engine_set_volume(&engine_, master_volume_);
}

}  // namespace audio