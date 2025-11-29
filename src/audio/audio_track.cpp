#define NOMINMAX  // Prevent Windows from defining min/max macros
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>

#include "audio_track.h"
#include "audio_system.h"
#include "sound.h"
#include "audio_manager.h"

using namespace std::chrono_literals;

namespace audio {

AudioTrack::AudioTrack(AudioSystem* system) 
    : system_(system), is_playing_(false) {
}

AudioTrack::~AudioTrack() {
  Stop();
}

void AudioTrack::AddLayer(const std::string& name, const std::string& filepath, 
                         AudioGroup* group, bool looping) {
  Layer layer;
  layer.sound = system_->CreateSound(filepath, group);
  layer.sound->SetLooping(looping);
  layer.current_volume = 0.0f;  // Start silent
  layer.sound->SetVolume(0.0f);
  layer.is_fading = false;
  layers_[name] = std::move(layer);
}

void AudioTrack::RemoveLayer(const std::string& name) {
  auto it = layers_.find(name);
  if (it == layers_.end()) return;

  // Stop the sound before removing
  it->second.sound->Stop();
  layers_.erase(it);
}

void AudioTrack::FadeLayer(const std::string& name, float target_volume,
                          std::chrono::milliseconds duration) {
  auto it = layers_.find(name);
  if (it == layers_.end()) {
    return;
  }

  Layer& layer = it->second;
  layer.start_volume = layer.current_volume;  // Remember where we started
  layer.target_volume = std::min(1.0f, std::max(0.0f, target_volume));  // Clamp target volume
  layer.fade_duration = duration;
  layer.fade_start_time = std::chrono::steady_clock::now();
  layer.fade_end_time = layer.fade_start_time + duration;
  layer.is_fading = true;
}

void AudioTrack::SetLayerVolume(const std::string& name, float volume) {
  auto it = layers_.find(name);
  if (it == layers_.end()) {
    return;
  }

  // Clamp volume between 0 and 1
  float clamped_volume = std::min(1.0f, std::max(0.0f, volume));
  
  Layer& layer = it->second;
  layer.current_volume = clamped_volume;
  layer.target_volume = clamped_volume;
  layer.is_fading = false;
  
  layer.sound->SetVolume(clamped_volume);
}

float AudioTrack::GetLayerVolume(const std::string& name) const {
  auto it = layers_.find(name);
  if (it == layers_.end()) return 0.0f;
  return it->second.current_volume;
}

void AudioTrack::Play() {
  if (is_playing_) return;
  
  // Start all sounds - they'll be in sync because they start together
  for (auto& pair : layers_) {
    pair.second.sound->Play();
  }
  is_playing_ = true;
}

void AudioTrack::Stop() {
  for (auto& pair : layers_) {
    pair.second.sound->Stop();
  }
  is_playing_ = false;
}

void AudioTrack::Update() {
  auto now = std::chrono::steady_clock::now();
  
  for (auto& pair : layers_) {
    Layer& layer = pair.second;
    if (!layer.is_fading) continue;

    if (now >= layer.fade_end_time) {
      // Fading finished
      layer.current_volume = layer.target_volume;
      layer.sound->SetVolume(layer.current_volume);  // Make sure we set the final volume
      layer.is_fading = false;
    } else {
      // Still fading - calculate progress as a percentage (0 to 1)
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          now - layer.fade_start_time);
      float progress = static_cast<float>(elapsed.count()) / layer.fade_duration.count();
      progress = (std::min)(1.0f, (std::max)(0.0f, progress));  // Clamp between 0 and 1
      
      // Linear interpolation between start and target volume
      float new_volume = layer.start_volume + 
          (layer.target_volume - layer.start_volume) * progress;
      
      // Only update if volume has changed
      if (new_volume != layer.current_volume) {
        layer.current_volume = new_volume;
        layer.sound->SetVolume(layer.current_volume);
      }
      

    }
    
    layer.sound->SetVolume(layer.current_volume);
  }
}

}  // namespace audio