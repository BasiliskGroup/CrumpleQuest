#ifndef AUDIO_SOUND_H_
#define AUDIO_SOUND_H_

#include "miniaudio/miniaudio.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @file sound.h
 * @brief Definition of the Sound class for individual audio file playback
 */

namespace audio {

// Forward declarations
class AudioGroup;

/**
 * @struct SoundInstance
 * @brief Represents a single instance of a playing sound
 * 
 * Since sounds can be played multiple times simultaneously, each playback
 * is represented by a SoundInstance object.
 */
struct SoundInstance {
  ma_sound* sound;       ///< Pointer to miniaudio sound
  bool finished;         ///< Flag indicating if this instance has finished playing
  
  /**
   * @brief Constructor
   */
  SoundInstance();
  
  /**
   * @brief Destructor
   * 
   * Cleans up miniaudio resources
   */
  ~SoundInstance();
};

/**
 * @class Sound
 * @brief Represents an audio file that can be played multiple times simultaneously
 * 
 * The Sound class wraps a single audio file but allows it to be played
 * multiple times concurrently (e.g., multiple footsteps from the same sound file).
 * Each playback instance is tracked separately and can be controlled individually.
 */
class Sound {
  /**
   * @brief Allow AudioManager to access private members
   */
  friend class AudioManager;
  
  /**
   * @brief Allow AudioSystem to create sounds
   */
  friend class AudioSystem;
  
  /**
   * @brief Allow AudioTrack to control playback
   */
  friend class AudioTrack;
 
 public:
  /**
   * @brief Factory method for creating Sound instances
   * 
   * This static factory method ensures proper initialization and
   * encapsulation of Sound objects.
   * 
   * @param engine Pointer to the miniaudio engine
   * @param filepath Path to the audio file
   * @param group Optional audio group this sound belongs to
   * @return std::unique_ptr<Sound> Unique pointer to a new Sound
   */
  static std::unique_ptr<Sound> Create(ma_engine* engine, const std::string& filepath, AudioGroup* group = nullptr) {
    return std::unique_ptr<Sound>(new Sound(engine, filepath, group));
  }
 
 private:
  /**
   * @brief Private constructor - use Create() instead
   * 
   * @param engine Pointer to the miniaudio engine
   * @param filepath Path to the audio file
   * @param group Optional audio group this sound belongs to
   */
  Sound(ma_engine* engine, const std::string& filepath, AudioGroup* group);
 
 public:
  /**
   * @brief Destructor
   * 
   * Stops all playing instances and releases resources
   */
  ~Sound();

 private:
  ///@name Playback Control
  ///@{
  
  /**
   * @brief Starts a new instance of the sound
   * 
   * Creates and plays a new instance of this sound, allowing
   * multiple overlapping playbacks of the same sound.
   */
  void Play();
  
  /**
   * @brief Stops all instances of this sound
   * 
   * Stops all currently playing instances of this sound.
   */
  void Stop();
  
  /**
   * @brief Sets whether new instances should loop
   * 
   * @param should_loop Whether new instances should loop continuously
   */
  void SetLooping(bool should_loop);
  
  /**
   * @brief Sets volume for all instances
   * 
   * @param volume Volume level (0.0 to 1.0)
   */
  void SetVolume(float volume);
  
  /**
   * @brief Sets pitch for the next instance to be played
   * 
   * @param pitch Pitch multiplier (1.0 = normal pitch, 0.5 = half speed, 2.0 = double speed)
   */
  void SetPitch(float pitch);
  ///@}

  ///@name State Queries
  ///@{
  
  /**
   * @brief Gets the current volume level
   * 
   * @return float Current volume (0.0 to 1.0)
   */
  float GetVolume() const { return volume_; }
  
  /**
   * @brief Checks if this sound is set to loop
   * 
   * @return bool True if set to loop, false otherwise
   */
  bool IsLooping() const { return looping_; }
  
  /**
   * @brief Checks if any instance of this sound is playing
   * 
   * @return bool True if any instance is playing, false otherwise
   */
  bool IsPlaying() const;
  ///@}

 private:
  /**
   * @brief Removes and deletes finished instances
   * 
   * Cleans up instances that have finished playing to free resources.
   */
  void CleanupFinishedInstances();
  
  std::vector<std::unique_ptr<SoundInstance>> sound_instances_; ///< Collection of playing instances
  ma_engine* engine_;                                ///< Pointer to miniaudio engine
  ma_sound_group* group_;                            ///< Pointer to parent sound group (if any)
  std::string filepath_;                             ///< Path to the audio file
  bool looping_;                                     ///< Whether new instances should loop
  float volume_;                                     ///< Current volume level
  float pitch_;                                      ///< Pitch for next instance (1.0 = normal)
};

}  // namespace audio

#endif  // AUDIO_SOUND_H_