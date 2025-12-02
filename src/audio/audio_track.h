#ifndef AUDIO_AUDIO_TRACK_H_
#define AUDIO_AUDIO_TRACK_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>

/**
 * @file audio_track.h
 * @brief Definition of the AudioTrack class for layered music
 */

namespace audio {

// Forward declarations
class Sound;
class AudioSystem;
class AudioGroup;

/**
 * @class AudioTrack
 * @brief Manages a collection of synchronized audio layers
 * 
 * The AudioTrack class provides functionality for layered music playback
 * where multiple audio tracks can play simultaneously and be independently
 * controlled. Each layer can be faded in or out independently, allowing
 * for dynamic music transitions based on gameplay.
 * 
 * Layers within a track are kept in sync, making them ideal for
 * musical stems that need to play together.
 */
class AudioTrack {
  /**
   * @brief AudioManager is the only class that can create AudioTracks
   */
  friend class AudioManager;
 
 public:
  /**
   * @brief Factory method for creating AudioTrack instances
   * 
   * This static factory method ensures proper initialization and
   * encapsulation of AudioTrack objects.
   * 
   * @param system Pointer to the audio system
   * @return std::unique_ptr<AudioTrack> Unique pointer to a new AudioTrack
   */
  static std::unique_ptr<AudioTrack> Create(AudioSystem* system) {
    return std::unique_ptr<AudioTrack>(new AudioTrack(system));
  }
 
 private:
  /**
   * @brief Private constructor - use Create() instead
   * 
   * @param system Pointer to the audio system
   */
  explicit AudioTrack(AudioSystem* system);
 
 public:
  /**
   * @brief Destructor
   * 
   * Stops all audio playback and releases resources
   */
  ~AudioTrack();
  
 private:

  /**
   * @brief Adds a new layer to the track
   * 
   * Each layer is a separate audio file that will play simultaneously
   * with other layers in the track. Layers are kept in sync.
   * 
   * @param name Identifier for the layer
   * @param filepath Path to the audio file
   * @param group Optional pointer to the group this layer belongs to
   * @param looping Whether the layer should loop continuously
   */
  void AddLayer(const std::string& name, const std::string& filepath, 
                AudioGroup* group = nullptr, bool looping = true);

  /**
   * @brief Removes a layer from the track
   * 
   * @param name Name of the layer to remove
   */
  void RemoveLayer(const std::string& name);

  /**
   * @brief Sets the target volume for a layer and fades to it over time
   * 
   * @param name Name of the layer
   * @param target_volume Target volume (0.0 to 1.0)
   * @param duration Duration of the fade in milliseconds
   */
  void FadeLayer(const std::string& name, float target_volume, 
                 std::chrono::milliseconds duration);
  
  /**
   * @brief Sets layer volume immediately without fading
   * 
   * @param name Name of the layer
   * @param volume Volume level (0.0 to 1.0)
   */
  void SetLayerVolume(const std::string& name, float volume);
  
  /**
   * @brief Gets current volume of a layer
   * 
   * @param name Name of the layer
   * @return float Current volume level (0.0 to 1.0)
   */
  float GetLayerVolume(const std::string& name) const;

  /**
   * @brief Starts playing all layers in sync
   * 
   * All layers will begin playback simultaneously to ensure
   * they remain synchronized.
   */
  void Play();
  
  /**
   * @brief Stops all layers
   * 
   * Stops playback of all layers in the track.
   */
  void Stop();
  
  /**
   * @brief Updates layer fading
   * 
   * This method is called regularly by the audio system
   * to update volume fading for all layers.
   */
  void Update();

 private:
  /**
   * @struct Layer
   * @brief Represents a single audio layer within a track
   * 
   * A layer contains a sound and data for controlling its playback
   * and volume transitions.
   */
  struct Layer {
    std::unique_ptr<Sound> sound;                        ///< The actual sound object
    float current_volume;                                ///< Current volume level
    float target_volume;                                 ///< Target volume for fading
    float start_volume;                                  ///< Volume when fade started
    std::chrono::steady_clock::time_point fade_start_time; ///< When fade began
    std::chrono::steady_clock::time_point fade_end_time;   ///< When fade will end
    std::chrono::milliseconds fade_duration;             ///< Duration of current fade
    bool is_fading;                                      ///< Whether this layer is currently fading

    /**
     * @brief Default constructor for Layer
     */
    Layer() : current_volume(1.0f), target_volume(1.0f),
              start_volume(1.0f), is_fading(false),
              fade_duration(std::chrono::milliseconds(0)) {}
  };

  AudioSystem* system_;                           ///< Reference to the audio system
  std::unordered_map<std::string, Layer> layers_; ///< Map of layer names to layer data
  bool is_playing_;                               ///< Whether the track is currently playing
};

}  // namespace audio

#endif  // AUDIO_AUDIO_TRACK_H_