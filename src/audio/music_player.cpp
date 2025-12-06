#include "audio/music_player.h"
#include <iostream>
#include <chrono>

namespace audio {

MusicPlayer::MusicPlayer() 
    : music_group_(0), 
      initialized_(false),
      parchment_track_(0),
      notebook_track_(0),
      grid_track_(0) {
}

MusicPlayer& MusicPlayer::Get() {
    static MusicPlayer instance; // Meyers singleton - thread-safe in C++11+
    return instance;
}

void MusicPlayer::Initialize(GroupHandle music_group) {
    if (initialized_) {
        return; // Already initialized
    }
    
    music_group_ = music_group;
    initialized_ = true;
    
    AudioManager& audio = AudioManager::GetInstance();
    
    // Create and setup parchment track
    parchment_track_ = audio.CreateTrack();
    audio.AddLayer(parchment_track_, "parchment", "sounds/parchment.wav", "music");
    audio.SetLayerVolume(parchment_track_, "parchment", 0.0f);
    audio.PlayTrack(parchment_track_);
    
    // Create and setup notebook track
    notebook_track_ = audio.CreateTrack();
    audio.AddLayer(notebook_track_, "notebook", "sounds/notebook.wav", "music");
    audio.SetLayerVolume(notebook_track_, "notebook", 1.0f);
    audio.PlayTrack(notebook_track_);
    
    // Create and setup grid track
    grid_track_ = audio.CreateTrack();
    audio.AddLayer(grid_track_, "grid", "sounds/grid.wav", "music");
    audio.SetLayerVolume(grid_track_, "grid", 0.0f);
    audio.PlayTrack(grid_track_);
}

void MusicPlayer::FadeTo(const std::string& track_name, float fade_duration) {
    if (!initialized_) {
        return;
    }
    
    AudioManager& audio = AudioManager::GetInstance();
    
    // Determine which track to fade to
    TrackHandle target_track = 0;
    if (track_name == "parchment") {
        target_track = parchment_track_;
    } else if (track_name == "notebook") {
        target_track = notebook_track_;
    } else if (track_name == "grid") {
        target_track = grid_track_;
    } else {
        return; // Unknown track
    }
    
    // Convert duration to milliseconds
    auto fade_ms = std::chrono::milliseconds(static_cast<long long>(fade_duration * 1000));
    
    // Fade out all other tracks
    if (target_track != parchment_track_) {
        audio.FadeLayer(parchment_track_, "parchment", 0.0f, fade_ms);
    }
    if (target_track != notebook_track_) {
        audio.FadeLayer(notebook_track_, "notebook", 0.0f, fade_ms);
    }
    if (target_track != grid_track_) {
        audio.FadeLayer(grid_track_, "grid", 0.0f, fade_ms);
    }
    
    // Fade in the target track
    audio.FadeLayer(target_track, track_name, 1.0f, fade_ms);
}

void MusicPlayer::SetTrackVolume(const std::string& track_name, float volume) {
    if (!initialized_) {
        return;
    }
    
    AudioManager& audio = AudioManager::GetInstance();
    
    if (track_name == "parchment") {
        audio.SetLayerVolume(parchment_track_, "parchment", volume);
    } else if (track_name == "notebook") {
        audio.SetLayerVolume(notebook_track_, "notebook", volume);
    } else if (track_name == "grid") {
        audio.SetLayerVolume(grid_track_, "grid", volume);
    }
}

void MusicPlayer::StopAll() {
    if (!initialized_) {
        return;
    }
    
    AudioManager& audio = AudioManager::GetInstance();
    
    audio.StopTrack(parchment_track_);
    audio.StopTrack(notebook_track_);
    audio.StopTrack(grid_track_);
}

} // namespace audio
