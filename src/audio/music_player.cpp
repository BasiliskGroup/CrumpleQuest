#include "audio/music_player.h"
#include <iostream>
#include <chrono>

namespace audio {

MusicPlayer::MusicPlayer() 
    : music_group_(0), 
      initialized_(false),
      restart_on_transition_(true), // Default to restarting tracks
      parchment_track_(0),
      notebook_track_(0),
      grid_track_(0),
      current_track_("grid") {
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
    audio.SetLayerVolume(notebook_track_, "notebook", 0.0f);
    audio.PlayTrack(notebook_track_);
    
    // Create and setup grid track
    grid_track_ = audio.CreateTrack();
    audio.AddLayer(grid_track_, "grid", "sounds/grid.wav", "music");
    audio.SetLayerVolume(grid_track_, "grid", 1.0f);
    audio.PlayTrack(grid_track_);
    
    current_track_ = "grid";
}

TrackHandle MusicPlayer::GetTrackHandle(const std::string& track_name) {
    if (track_name == "parchment") {
        return parchment_track_;
    } else if (track_name == "notebook") {
        return notebook_track_;
    } else if (track_name == "grid") {
        return grid_track_;
    }
    return 0;
}

void MusicPlayer::FadeTo(const std::string& track_name, float fade_duration) {
    if (!initialized_) {
        return;
    }
    
    // Don't do anything if we're already on this track
    if (track_name == current_track_) {
        return;
    }
    
    AudioManager& audio = AudioManager::GetInstance();
    
    TrackHandle target_track = GetTrackHandle(track_name);
    if (target_track == 0) {
        return; // Unknown track
    }
    
    // Convert duration to milliseconds
    auto fade_ms = std::chrono::milliseconds(static_cast<long long>(fade_duration * 1000));
    
    if (restart_on_transition_) {
        // Stop and restart behavior
        
        // Fade out current track, then stop it
        TrackHandle current_track_handle = GetTrackHandle(current_track_);
        if (current_track_handle != 0) {
            audio.FadeLayer(current_track_handle, current_track_, 0.0f, fade_ms);
            // Note: We can't easily stop the track after the fade completes without a callback system
            // For now, leaving it at 0 volume is sufficient
        }
        
        // Start the target track from the beginning and fade it in
        audio.StopTrack(target_track);
        audio.PlayTrack(target_track);
        audio.SetLayerVolume(target_track, track_name, 0.0f);
        audio.FadeLayer(target_track, track_name, 1.0f, fade_ms);
    } else {
        // Continuous playback behavior - all tracks play continuously
        
        // Make sure all tracks are playing (calling PlayTrack on already playing track is safe)
        audio.PlayTrack(parchment_track_);
        audio.PlayTrack(notebook_track_);
        audio.PlayTrack(grid_track_);
        
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
    
    current_track_ = track_name;
}

void MusicPlayer::SetTrackVolume(const std::string& track_name, float volume) {
    if (!initialized_) {
        return;
    }
    
    AudioManager& audio = AudioManager::GetInstance();
    
    TrackHandle track = GetTrackHandle(track_name);
    if (track != 0) {
        audio.SetLayerVolume(track, track_name, volume);
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
