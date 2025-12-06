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
    
    // Track handles
    TrackHandle parchment_track_;
    TrackHandle notebook_track_;
    TrackHandle grid_track_;
    
    // Private constructor for singleton
    MusicPlayer();
    
    // Delete copy and move
    MusicPlayer(const MusicPlayer&) = delete;
    MusicPlayer& operator=(const MusicPlayer&) = delete;
    MusicPlayer(MusicPlayer&&) = delete;
    MusicPlayer& operator=(MusicPlayer&&) = delete;

public:
    ~MusicPlayer() = default;
    
    // Singleton access (Meyers singleton - thread-safe, automatic cleanup)
    static MusicPlayer& Get();
    
    // Initialize with music group (must be called before first use)
    void Initialize(GroupHandle music_group);
    
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
