#ifndef AUDIO_AUDIO_SYSTEM_H_
#define AUDIO_AUDIO_SYSTEM_H_

#include "miniaudio/miniaudio.h"
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @file audio_system.h
 * @brief Definition of the AudioSystem class for low-level audio operations
 */

namespace audio {

// Forward declarations
class AudioGroup;
class Sound;

/**
 * @class AudioSystem
 * @brief Low-level audio system that interfaces directly with miniaudio
 * 
 * The AudioSystem class handles the core audio functionality, including
 * initialization of the miniaudio engine and creation of sounds and groups.
 * It manages the underlying audio resources and provides a layer of
 * abstraction over the miniaudio library.
 * 
 * AudioSystem is not meant to be used directly by game code; instead,
 * it's accessed through the AudioManager.
 */
class AudioSystem {
 public:
  /**
   * @brief Constructor
   * 
   * Initializes the miniaudio engine
   */
  AudioSystem();
  
  /**
   * @brief Destructor
   * 
   * Cleans up all audio resources and uninitializes the miniaudio engine
   */
  ~AudioSystem();

  /**
   * @brief Creates a new Sound object
   * 
   * Loads an audio file and creates a Sound object. The caller takes ownership
   * of the returned unique_ptr.
   * 
   * @param filepath Path to the audio file
   * @param group Optional pointer to the group to add the sound to
   * @return std::unique_ptr<Sound> Unique pointer to the newly created Sound (caller owns)
   */
  std::unique_ptr<Sound> CreateSound(const std::string& filepath, AudioGroup* group = nullptr);

  /**
   * @brief Creates a new AudioGroup object
   * 
   * Creates an audio group for collective control of sounds. The caller takes
   * ownership of the returned unique_ptr.
   * 
   * @param name Name for the group
   * @return std::unique_ptr<AudioGroup> Unique pointer to the newly created AudioGroup (caller owns)
   */
  std::unique_ptr<AudioGroup> CreateGroup(const std::string& name);

  /**
   * @brief Sets the master volume for all audio
   * 
   * @param volume Volume level (0.0 to 1.0)
   */
  void SetMasterVolume(float volume);
  
  /**
   * @brief Gets the current master volume
   * 
   * @return float Current master volume (0.0 to 1.0)
   */
  float GetMasterVolume() const { return master_volume_; }

 private:
  ma_engine engine_;                                      ///< miniaudio engine instance
  float master_volume_;                                   ///< Master volume level
};

}  // namespace audio

#endif  // AUDIO_AUDIO_SYSTEM_H_