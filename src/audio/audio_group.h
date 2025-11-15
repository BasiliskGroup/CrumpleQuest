#ifndef AUDIO_AUDIO_GROUP_H_
#define AUDIO_AUDIO_GROUP_H_

#include "miniaudio.h"
#include <chrono>
#include <memory>

/**
 * @file audio_group.h
 * @brief Definition of the AudioGroup class for grouped audio control
 */

namespace audio {

/**
 * @class AudioGroup
 * @brief Controls multiple sounds as a single unit
 * 
 * AudioGroup allows collective control over multiple sounds (e.g., all music 
 * or all sound effects). This enables group volume adjustment, fading, and
 * other operations to be applied to multiple sounds simultaneously.
 * 
 * Groups are mapped to miniaudio sound groups internally.
 */
class AudioGroup {
  /**
   * @brief Allow AudioManager to access private members for control
   */
  friend class AudioManager;
  
  /**
   * @brief Allow AudioSystem to create groups
   */
  friend class AudioSystem;
  
  /**
   * @brief Allow Sound to access the group handle
   */
  friend class Sound;
 
 public:
  /**
   * @brief Factory method for creating AudioGroup instances
   * 
   * This static factory method ensures proper initialization and
   * encapsulation of AudioGroup objects.
   * 
   * @param engine Pointer to the miniaudio engine
   * @return std::unique_ptr<AudioGroup> Unique pointer to a new AudioGroup
   */
  static std::unique_ptr<AudioGroup> Create(ma_engine* engine) {
    return std::unique_ptr<AudioGroup>(new AudioGroup(engine));
  }
 
 private:
  /**
   * @brief Private constructor - use Create() instead
   * 
   * @param engine Pointer to the miniaudio engine
   */
  explicit AudioGroup(ma_engine* engine);
 
 public:
  /**
   * @brief Destructor
   * 
   * Cleans up miniaudio resources
   */
  ~AudioGroup();
  
 private:

  /**
   * @brief Sets the volume for this group
   * 
   * @param volume Volume level (0.0 to 1.0)
   */
  void SetVolume(float volume);
  
  /**
   * @brief Gets the current volume for this group
   * 
   * @return float Current volume level (0.0 to 1.0)
   */
  float GetVolume() const { return volume_; }

  /**
   * @brief Fades the group volume to a target over duration
   * 
   * @param targetVolume Target volume level (0.0 to 1.0)
   * @param duration Duration of the fade in milliseconds
   */
  void FadeVolume(float targetVolume, std::chrono::milliseconds duration);

  /**
   * @brief Gets the underlying miniaudio sound group handle
   * 
   * @return ma_sound_group* Pointer to the miniaudio sound group
   */
  ma_sound_group* GetHandle() { return sound_group_; }

  /**
   * @brief Gets the end time of the current fade
   * 
   * @return std::chrono::steady_clock::time_point When the fade will complete
   */
  std::chrono::steady_clock::time_point GetFadeEndTime() const { return fade_end_time_; }
  
  /**
   * @brief Gets the target volume for the current fade
   * 
   * @return float Target volume level
   */
  float GetTargetVolume() const { return target_volume_; }
  
  /**
   * @brief Gets the starting volume for the current fade
   * 
   * @return float Starting volume level
   */
  float GetStartVolume() const { return start_volume_; }
  
  /**
   * @brief Checks if the group is currently fading
   * 
   * @return bool True if currently fading, false otherwise
   */
  bool IsFading() const { return is_fading_; }
  
  /**
   * @brief Gets the duration of the current fade
   * 
   * @return std::chrono::milliseconds Duration in milliseconds
   */
  std::chrono::milliseconds GetFadeDuration() const { return fade_duration_; }
  
  /**
   * @brief Allows AudioManager to set fading state directly
   * 
   * @param fading Whether the group is fading
   */
  void SetFadingState(bool fading) { is_fading_ = fading; }

 private:
  ///@name Fade State
  ///@{
  float target_volume_{0.0f};                          ///< Target volume for fade
  float start_volume_{0.0f};                           ///< Starting volume for fade
  bool is_fading_{false};                              ///< Whether currently fading
  std::chrono::steady_clock::time_point fade_end_time_; ///< When the fade will end
  std::chrono::milliseconds fade_duration_{0};          ///< Duration of the fade
  ///@}
  
  ///@name miniaudio Resources
  ///@{
  ma_sound_group* sound_group_;                        ///< miniaudio sound group handle
  ma_engine* engine_;                                  ///< Reference to miniaudio engine
  ///@}
  
  float volume_;                                       ///< Current volume level
};

}  // namespace audio

#endif  // AUDIO_AUDIO_GROUP_H_