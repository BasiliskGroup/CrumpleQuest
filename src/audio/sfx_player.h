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

    // Helper to load a collection
    void LoadCollection(const std::string& name, 
                       const std::string& folder_path,
                       const RandomSoundContainerConfig& config);

public:
    /**
     * @brief Construct and initialize all sound collections
     * @param sfx_group The audio group handle for all SFX
     */
    explicit SFXPlayer(GroupHandle sfx_group);
    ~SFXPlayer() = default;

    /**
     * @brief Play a sound effect
     * @param sfx_name Name of the SFX to play (e.g., "fold", "flip", "menu_touch")
     */
    void Play(const std::string& sfx_name);
};

} // namespace audio

#endif // SFX_PLAYER_H
