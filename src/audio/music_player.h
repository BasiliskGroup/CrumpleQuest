#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "audio/audio_manager.h"
#include <string>
#include <unordered_map>

namespace audio {

/**
 * @brief Manages and controls all game music tracks
 * 
 * Central manager for music playback and transitions. Handles loading
 * music tracks and smooth transitions between different game states.
 */
class MusicPlayer {
private:
    GroupHandle music_group_;
    bool initialized_;
    bool restart_on_transition_; // Whether to restart tracks from beginning on transition
    
    // Track handles
    TrackHandle parchment_track_;
    TrackHandle notebook_track_;
    TrackHandle grid_track_;
    TrackHandle boss_track_;
    
    // Current active track name
    std::string current_track_;
    
    // Private constructor for singleton
    MusicPlayer();
    
    // Delete copy and move
    MusicPlayer(const MusicPlayer&) = delete;
    MusicPlayer& operator=(const MusicPlayer&) = delete;
    MusicPlayer(MusicPlayer&&) = delete;
    MusicPlayer& operator=(MusicPlayer&&) = delete;
    
    // Get track handle by name
    TrackHandle GetTrackHandle(const std::string& track_name);

public:
    ~MusicPlayer() = default;
    
    // Singleton access (Meyers singleton - thread-safe, automatic cleanup)
    static MusicPlayer& Get();
    
    // Initialize with music group (must be called before first use)
    void Initialize(GroupHandle music_group);
    
    /**
     * @brief Set whether tracks should restart from beginning on transition
     * @param restart If true, tracks restart from beginning; if false, they continue from current position
     */
    void SetRestartOnTransition(bool restart) { restart_on_transition_ = restart; }
    
    /**
     * @brief Get whether tracks restart from beginning on transition
     */
    bool GetRestartOnTransition() const { return restart_on_transition_; }
    
    /**
     * @brief Get the name of the currently playing track
     * @return Name of the current track (empty string if none)
     */
    std::string GetCurrentTrack() const { return current_track_; }
    
    /**
     * @brief Fade to a specific music track
     * @param track_name Name of the track ("parchment", "notebook", "grid")
     * @param fade_duration Duration of the fade in seconds
     */
    void FadeTo(const std::string& track_name, float fade_duration = 1.0f);
    
    /**
     * @brief Set volume for a specific track
     * @param track_name Name of the track
     * @param volume Volume level (0.0 to 1.0)
     */
    void SetTrackVolume(const std::string& track_name, float volume);
    
    /**
     * @brief Stop all music tracks
     */
    void StopAll();
};

} // namespace audio

#endif // MUSIC_PLAYER_H
