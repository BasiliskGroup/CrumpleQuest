#define NOMINMAX  // Prevent Windows from defining min/max macros
#include "audio_group.h"
#include <algorithm>
#include <iostream>

namespace audio {

AudioGroup::AudioGroup(ma_engine* engine) 
    : engine_(engine), volume_(1.0f), is_fading_(false) {
  sound_group_ = new ma_sound_group;
  if (ma_sound_group_init(engine_, 0, nullptr, sound_group_) != MA_SUCCESS) {
    std::cerr << "Failed to initialize sound group" << std::endl;
    delete sound_group_;
    sound_group_ = nullptr;
  }
}

AudioGroup::~AudioGroup() {
  if (sound_group_) {
    ma_sound_group_uninit(sound_group_);
    delete sound_group_;
    sound_group_ = nullptr;
  }
}

void AudioGroup::SetVolume(float volume) {
  // Clamp volume between 0 and 1
  volume_ = std::min(1.0f, std::max(0.0f, volume));
            
  if (sound_group_) {
    ma_sound_group_set_volume(sound_group_, volume_);
  }
}

void AudioGroup::FadeVolume(float targetVolume, std::chrono::milliseconds duration) {
  // Clamp target volume between 0 and 1
  target_volume_ = std::min(1.0f, std::max(0.0f, targetVolume));
  
  // Save current volume as starting point
  start_volume_ = volume_;
  
  // Setup fade timing
  fade_duration_ = duration;
  is_fading_ = true;
  fade_end_time_ = std::chrono::steady_clock::now() + duration;
  
  std::cout << "Starting group volume fade from " << start_volume_ 
            << " to " << target_volume_ 
            << " over " << duration.count() << "ms" << std::endl;
}

}  // namespace audio