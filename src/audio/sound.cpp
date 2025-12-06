#define NOMINMAX  // Prevent Windows from defining min/max macros
#include "sound.h"
#include "audio_group.h"
#include <iostream>
#include <algorithm>

namespace audio {

SoundInstance::SoundInstance() : sound(nullptr), finished(false) {}

SoundInstance::~SoundInstance() {
  if (sound) {
    ma_sound_uninit(sound);
    delete sound;
  }
}

Sound::Sound(ma_engine* engine, const std::string& filepath, AudioGroup* group) 
    : engine_(engine), 
      filepath_(filepath), 
      looping_(false), 
      volume_(1.0f),
      pitch_(1.0f),
      group_(group ? group->GetHandle() : nullptr) {
}

Sound::~Sound() {
  Stop();  // Stops and cleans up all instances
}

void Sound::CleanupFinishedInstances() {
  // Remove any instances that have finished playing
  auto it = sound_instances_.begin();
  while (it != sound_instances_.end()) {
    if ((*it)->sound && ma_sound_is_playing((*it)->sound) == MA_FALSE) {
      it = sound_instances_.erase(it);  // Smart pointer handles cleanup
    } else {
      ++it;
    }
  }
}

void Sound::Play() {
  // First cleanup any finished instances
  CleanupFinishedInstances();
  
  // If looping and already playing, don't create a new instance
  if (looping_ && !sound_instances_.empty()) {
    // Just ensure the existing instance is playing
    for (auto& instance : sound_instances_) {
      if (instance->sound && ma_sound_is_playing(instance->sound) == MA_FALSE) {
        ma_sound_start(instance->sound);
      }
    }
    return;
  }
  
  // Create new instance
  auto instance = std::make_unique<SoundInstance>();
  instance->sound = new ma_sound;
  
  // Stream large music files, don't stream small SFX for better sync
  // Use streaming for files in music group or looping files (likely music)
  uint32_t flags = (group_ != nullptr || looping_) ? MA_SOUND_FLAG_STREAM : 0;
  std::cout << "[Sound::Play] Loading sound file: " << filepath_ << " (streaming: " << (flags ? "yes" : "no") << ")" << std::endl;
  ma_result result = ma_sound_init_from_file(
      engine_,
      filepath_.c_str(),
      flags,
      group_,
      nullptr,
      instance->sound);
      
  if (result != MA_SUCCESS) {
    std::cerr << "[Sound::Play] FAILED to load sound file: " << filepath_ << " (error code: " << result << ")" << std::endl;
    return;  // Instance will be cleaned up by smart pointer
  }
  std::cout << "[Sound::Play] Successfully loaded: " << filepath_ << std::endl;
  
  // Configure and play
  ma_sound_set_looping(instance->sound, looping_);
  ma_sound_set_volume(instance->sound, volume_);
  ma_sound_set_pitch(instance->sound, pitch_);
  ma_sound_start(instance->sound);
  
  // Add to our list of instances
  sound_instances_.push_back(std::move(instance));
}

void Sound::Stop() {
  for (auto& instance : sound_instances_) {
    if (instance->sound) {
      ma_sound_stop(instance->sound);
    }
  }
  sound_instances_.clear();  // Smart pointers handle cleanup
}

void Sound::SetLooping(bool should_loop) {
  looping_ = should_loop;
  for (auto& instance : sound_instances_) {
    if (instance->sound) {
      ma_sound_set_looping(instance->sound, looping_);
    }
  }
}

void Sound::SetVolume(float volume) {
  // Clamp volume between 0 and 1
  volume_ = (std::min)(1.0f, (std::max)(0.0f, volume));
  
  std::cout << "[Sound::SetVolume] " << filepath_ << " -> " << volume_ << " (instances: " << sound_instances_.size() << ")" << std::endl;
            
  for (auto& instance : sound_instances_) {
    if (instance->sound) {
      ma_sound_set_volume(instance->sound, volume_);
    }
  }
}

void Sound::SetPitch(float pitch) {
  pitch_ = (std::max)(0.1f, pitch);  // Clamp minimum to avoid issues
}

bool Sound::IsPlaying() const {
  // Return true if any instance is playing
  for (const auto& instance : sound_instances_) {
    if (instance->sound && ma_sound_is_playing(instance->sound) == MA_TRUE) {
      return true;
    }
  }
  return false;
}

}  // namespace audio