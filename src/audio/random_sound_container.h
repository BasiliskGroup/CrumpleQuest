#pragma once

#include <vector>
#include <string>
#include <random>
#include "audio_manager.h"

namespace audio {

/**
 * @struct RandomSoundContainerConfig
 * @brief Configuration for a random sound container
 */
struct RandomSoundContainerConfig {
    bool avoidRepeat = true;           ///< Avoid playing the same sound twice in a row
    float pitchMin = 1.0f;             ///< Minimum pitch shift (1.0 = normal pitch)
    float pitchMax = 1.0f;             ///< Maximum pitch shift (1.0 = normal pitch)
    GroupHandle group = 0;             ///< Audio group to assign sounds to
};

/**
 * @class RandomSoundContainer
 * @brief Container for playing randomized sounds with pitch variation
 * 
 * Similar to Wwise random containers, this class manages a collection of sounds
 * and plays them randomly with optional pitch variation and repeat avoidance.
 */
class RandomSoundContainer {
public:
    /**
     * @brief Construct a random sound container
     * 
     * @param name Name identifier for this container
     * @param config Configuration settings
     */
    RandomSoundContainer(const std::string& name, const RandomSoundContainerConfig& config = {});
    
    /**
     * @brief Destructor
     */
    ~RandomSoundContainer();
    
    /**
     * @brief Add a sound to the container
     * 
     * @param filepath Path to the audio file
     */
    void AddSound(const std::string& filepath);
    
    /**
     * @brief Load all .wav files from a folder
     * 
     * @param folderPath Path to the folder containing sound files
     */
    void LoadFromFolder(const std::string& folderPath);
    
    /**
     * @brief Play a random sound from the container
     * 
     * Applies pitch variation and avoids repeating the last played sound if configured.
     */
    void Play();
    
    /**
     * @brief Play a random sound with specific volume
     * 
     * @param volume Volume level (0.0 to 1.0)
     */
    void PlayWithVolume(float volume);
    
    /**
     * @brief Stop all currently playing sounds from this container
     */
    void StopAll();
    
    /**
     * @brief Set the pitch range for randomization
     * 
     * @param minPitch Minimum pitch multiplier (e.g., 0.9 for 10% lower)
     * @param maxPitch Maximum pitch multiplier (e.g., 1.1 for 10% higher)
     */
    void SetPitchRange(float minPitch, float maxPitch);
    
    /**
     * @brief Enable or disable repeat avoidance
     * 
     * @param avoid If true, the same sound won't play twice in a row
     */
    void SetAvoidRepeat(bool avoid);
    
    /**
     * @brief Get the name of this container
     * 
     * @return const std::string& Container name
     */
    const std::string& GetName() const { return name_; }
    
    /**
     * @brief Get the number of sounds in this container
     * 
     * @return size_t Number of loaded sounds
     */
    size_t GetSoundCount() const { return sounds_.size(); }

private:
    std::string name_;                          ///< Container name
    std::vector<SoundHandle> sounds_;           ///< Loaded sound handles
    RandomSoundContainerConfig config_;         ///< Configuration settings
    SoundHandle last_played_;                   ///< Handle of the last played sound
    std::mt19937 rng_;                          ///< Random number generator
    
    /**
     * @brief Select a random sound, avoiding the last played if configured
     * 
     * @return SoundHandle Handle to the selected sound
     */
    SoundHandle SelectRandomSound();
};

} // namespace audio
