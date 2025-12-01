#define NOMINMAX  // Prevent Windows from defining min/max macros
#include "audio_manager.h"
#include "audio_system.h"
#include "audio_track.h"
#include "audio_group.h"
#include "sound.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <iostream>
#include <algorithm>

// Place all std usage in global namespace
using ::std::lock_guard;
using ::std::mutex;
using ::std::thread;
using ::std::unique_ptr;
using ::std::shared_ptr;
using ::std::make_unique;
using ::std::make_shared;
using namespace ::std::chrono_literals;

namespace audio {

AudioManager::AudioManager() {
    audio_system_ = make_unique<AudioSystem>();
}

AudioManager::~AudioManager() {
    if (running_) {
        Shutdown();
    }
}

AudioManager& AudioManager::GetInstance() {
    static AudioManager instance;
    return instance;
}

bool AudioManager::Initialize() {
    if (running_) {
        std::cerr << "AudioManager already running" << std::endl;
        return false;
    }

    running_ = true;

    update_thread_ = thread([this]() {
        while (running_) {
            std::this_thread::sleep_for(16ms); // ~60Hz update rate

            lock_guard<mutex> lock(resource_mutex_);

            // Update all tracks
            for (auto& [handle, track] : tracks_) {
                if (track) {
                    track->Update();
                }
            }
            
            // Update all groups for fading
            auto now = std::chrono::steady_clock::now();
            for (auto& [handle, group] : groups_) {
                if (!group) continue;
                
                // Check if the group is fading
                auto fade_end_time = group->GetFadeEndTime();
                if (now < fade_end_time) {
                    // Calculate progress as a percentage (0 to 1)
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - (fade_end_time - group->GetFadeDuration()));
                    float progress = static_cast<float>(elapsed.count()) / group->GetFadeDuration().count();
                    progress = std::min(1.0f, std::max(0.0f, progress));
                    
                    // Linear interpolation between start and target volume
                    float new_volume = group->GetStartVolume() + 
                        (group->GetTargetVolume() - group->GetStartVolume()) * progress;
                    
                    group->SetVolume(new_volume);
                }
                else if (group->IsFading()) {
                    // Fade is complete, set to target volume
                    group->SetVolume(group->GetTargetVolume());
                    group->SetFadingState(false);
                }
            }
        }
    });

    return true;
}

void AudioManager::Shutdown() {
    if (!running_) return;

    running_ = false;
    if (update_thread_.joinable()) {
        update_thread_.join();
    }

    // Clean up resources in proper order
    lock_guard<mutex> lock(resource_mutex_);
    
    // First stop all tracks to ensure sounds stop before being destroyed
    for (auto& [handle, track] : tracks_) {
        if (track) {
            track->Stop();
        }
    }
    
    // Then clear all resources
    tracks_.clear();
    sounds_.clear();  // Clear sounds before groups
    groups_.clear();  // Clear groups before audio system
    group_names_.clear();

    // Audio system will be cleaned up by its destructor
}

// System control
void AudioManager::SetMasterVolume(float volume) {
    lock_guard<mutex> lock(resource_mutex_);
    if (audio_system_) {
        audio_system_->SetMasterVolume(volume);
    }
}

float AudioManager::GetMasterVolume() const {
    if (!audio_system_) return 0.0f;
    return audio_system_->GetMasterVolume();
}

// Track management
TrackHandle AudioManager::CreateTrack() {
    lock_guard<mutex> lock(resource_mutex_);
    
    // Create a new track handle
    TrackHandle handle = NextTrackHandle();
    
    // Create the track and store it
    tracks_[handle] = AudioTrack::Create(audio_system_.get());
    
    return handle;
}

void AudioManager::DestroyTrack(TrackHandle track) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = tracks_.find(track);
    if (it != tracks_.end()) {
        tracks_.erase(it);
    }
}

void AudioManager::PlayTrack(TrackHandle track) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = tracks_.find(track);
    if (it != tracks_.end() && it->second) {
        it->second->Play();
    }
}

void AudioManager::StopTrack(TrackHandle track) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = tracks_.find(track);
    if (it != tracks_.end() && it->second) {
        it->second->Stop();
    }
}

// Layer operations
void AudioManager::AddLayer(TrackHandle track, const string& layerName, 
                           const string& filepath, const string& group) {
    lock_guard<mutex> lock(resource_mutex_);
    auto track_it = tracks_.find(track);
    if (track_it == tracks_.end() || !track_it->second) return;
    
    track_it->second->AddLayer(layerName, filepath, group, true);
}

void AudioManager::RemoveLayer(TrackHandle track, const string& layerName) {
    lock_guard<mutex> lock(resource_mutex_);
    auto track_it = tracks_.find(track);
    if (track_it == tracks_.end() || !track_it->second) return;
    
    track_it->second->RemoveLayer(layerName);
}

void AudioManager::SetLayerVolume(TrackHandle track, const string& layerName, float volume) {
    lock_guard<mutex> lock(resource_mutex_);
    auto track_it = tracks_.find(track);
    if (track_it == tracks_.end() || !track_it->second) return;
    
    track_it->second->SetLayerVolume(layerName, volume);
}

void AudioManager::FadeLayer(TrackHandle track, const string& layerName, 
                            float targetVolume, std::chrono::milliseconds duration) {
    lock_guard<mutex> lock(resource_mutex_);
    auto track_it = tracks_.find(track);
    if (track_it == tracks_.end() || !track_it->second) return;
    
    track_it->second->FadeLayer(layerName, targetVolume, duration);
}

// Group operations
GroupHandle AudioManager::CreateGroup(const string& name) {
    lock_guard<mutex> lock(resource_mutex_);
    
    // Create a new group handle
    GroupHandle handle = NextGroupHandle();
    
    // Create the group in the audio system
    auto* group_ptr = audio_system_->CreateGroup(name);
    if (!group_ptr) return 0;
    
    // Store the group
    groups_[handle].reset(group_ptr);
    
    // Store name mapping if provided
    if (!name.empty()) {
        group_names_[name] = handle;
    }
    
    return handle;
}

void AudioManager::DestroyGroup(GroupHandle group) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = groups_.find(group);
    if (it != groups_.end()) {
        // Remove from name mapping if present
        for (auto name_it = group_names_.begin(); name_it != group_names_.end(); ) {
            if (name_it->second == group) {
                name_it = group_names_.erase(name_it);
            } else {
                ++name_it;
            }
        }
        groups_.erase(it);
    }
}

void AudioManager::SetGroupVolume(GroupHandle group, float volume) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = groups_.find(group);
    if (it != groups_.end() && it->second) {
        it->second->SetVolume(volume);
    }
}

float AudioManager::GetGroupVolume(GroupHandle group) const {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = groups_.find(group);
    if (it != groups_.end() && it->second) {
        return it->second->GetVolume();
    }
    return 0.0f;
}

void AudioManager::FadeGroup(GroupHandle group, float targetVolume, 
                            std::chrono::milliseconds duration) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = groups_.find(group);
    if (it != groups_.end() && it->second) {
        it->second->FadeVolume(targetVolume, duration);
    }
}

// Sound operations
SoundHandle AudioManager::LoadSound(const string& filepath) {
    lock_guard<mutex> lock(resource_mutex_);
    
    // Create a new sound handle
    SoundHandle handle = NextSoundHandle();
    
    // Create the sound
    Sound* sound_ptr = audio_system_->CreateSound(filepath);
    if (!sound_ptr) return 0;
    
    // Store the sound
    sounds_[handle] = shared_ptr<Sound>(sound_ptr);
    
    return handle;
}

void AudioManager::UnloadSound(SoundHandle sound) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = sounds_.find(sound);
    if (it != sounds_.end()) {
        sounds_.erase(it);
    }
}

void AudioManager::StartSound(SoundHandle sound) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = sounds_.find(sound);
    if (it != sounds_.end() && it->second) {
        it->second->Play();
    }
}

void AudioManager::StopSound(SoundHandle sound) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = sounds_.find(sound);
    if (it != sounds_.end() && it->second) {
        it->second->Stop();
    }
}

void AudioManager::SetSoundVolume(SoundHandle sound, float volume) {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = sounds_.find(sound);
    if (it != sounds_.end() && it->second) {
        it->second->SetVolume(volume);
    }
}

bool AudioManager::IsSoundPlaying(SoundHandle sound) const {
    lock_guard<mutex> lock(resource_mutex_);
    auto it = sounds_.find(sound);
    if (it != sounds_.end() && it->second) {
        return it->second->IsPlaying();
    }
    return false;
}

} // namespace audio