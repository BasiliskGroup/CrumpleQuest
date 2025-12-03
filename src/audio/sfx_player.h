#ifndef SFX_PLAYER_H
#define SFX_PLAYER_H

#include "audio/random_sound_container.h"
#include "audio/audio_manager.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace audio {

/**
 * @brief Manages and plays all game sound effects
 * 
 * Central manager for all SFX in the game. Internally creates and manages
 * all RandomSoundContainers needed for the game.
 */
class SFXPlayer {
private:
    GroupHandle sfx_group_;
    std::unordered_map<std::string, std::unique_ptr<RandomSoundContainer>> containers_;
    bool initialized_;

    // Private constructor for singleton
    SFXPlayer();
    
    // Delete copy and move
    SFXPlayer(const SFXPlayer&) = delete;
    SFXPlayer& operator=(const SFXPlayer&) = delete;
    SFXPlayer(SFXPlayer&&) = delete;
    SFXPlayer& operator=(SFXPlayer&&) = delete;
    
    // Helper to load a collection
    void LoadCollection(const std::string& name, 
                       const std::string& folder_path,
                       const RandomSoundContainerConfig& config);

public:
    ~SFXPlayer() = default;
    
    // Singleton access (Meyers singleton - thread-safe, automatic cleanup)
    static SFXPlayer& Get();
    
    // Initialize with SFX group (must be called before first use)
    void Initialize(GroupHandle sfx_group);

    /**
     * @brief Play a sound effect
     * @param sfx_name Name of the SFX to play (e.g., "fold", "flip", "menu_touch")
     */
    void Play(const std::string& sfx_name);
};

} // namespace audio

#endif // SFX_PLAYER_H
