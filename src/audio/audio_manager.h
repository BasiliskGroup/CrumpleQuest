#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <string>
#include <memory>
#include <vector>

/**
 * @file audio_manager.h
 * @brief Main interface for the audio system
 * 
 * This file contains the AudioManager class which serves as the primary
 * interface for all audio functionality in the game engine.
 */

// Forward declarations
namespace audio {
    class AudioSystem;
    class AudioTrack;
    class AudioGroup;
    class Sound;
}

// Place std type usage in global namespace
using ::std::atomic;
using ::std::mutex;
using ::std::thread;
using ::std::unordered_set;
using ::std::unordered_map;
using ::std::string;
using ::std::unique_ptr;
using ::std::shared_ptr;
using namespace ::std::chrono_literals;

namespace audio {

/**
 * @brief Opaque handle types for audio objects
 * 
 * These typedefs define opaque handles that are used to reference audio objects.
 * The actual implementation details are hidden from the user.
 */
using TrackHandle = uint32_t;   ///< Handle for referencing audio tracks
using GroupHandle = uint32_t;   ///< Handle for referencing audio groups
using SoundHandle = uint32_t;   ///< Handle for referencing individual sounds

/**
 * @class AudioManager
 * @brief Central manager for all audio functionality
 * 
 * The AudioManager class provides a singleton interface for all audio operations.
 * It manages audio tracks, groups, and individual sounds, providing a high-level
 * API for game code to interact with the audio system.
 * 
 * This class handles resource management, playback control, volume adjustments,
 * and other audio operations. It is the only class that should be directly
 * accessed by game code.
 */
class AudioManager {
public:
    /**
     * @brief Get the singleton instance of the AudioManager
     * 
     * @return AudioManager& Reference to the singleton instance
     */
    static AudioManager& GetInstance();

    ///@name System Lifecycle
    ///@{
    
    /**
     * @brief Initialize the audio system
     * 
     * This function initializes the underlying audio system, including
     * the miniaudio engine. It must be called before any other audio
     * operations.
     * 
     * @return bool True if initialization was successful, false otherwise
     */
    bool Initialize();
    
    /**
     * @brief Shut down the audio system
     * 
     * This function cleans up all audio resources and shuts down the
     * miniaudio engine. It should be called before application exit.
     */
    void Shutdown();
    ///@}

    ///@name System Control
    ///@{
    
    /**
     * @brief Set the master volume for all audio
     * 
     * @param volume Volume level (0.0 to 1.0)
     */
    void SetMasterVolume(float volume);
    
    /**
     * @brief Get the current master volume level
     * 
     * @return float Current master volume (0.0 to 1.0)
     */
    float GetMasterVolume() const;
    ///@}

    ///@name Track Management
    ///@{
    
    /**
     * @brief Create a new audio track
     * 
     * Audio tracks can contain multiple layers of audio that play
     * simultaneously, with individual volume control for each layer.
     * 
     * @return TrackHandle Handle to the newly created track
     */
    TrackHandle CreateTrack();
    
    /**
     * @brief Destroy an audio track
     * 
     * This releases all resources associated with the track.
     * 
     * @param track Handle to the track to destroy
     */
    void DestroyTrack(TrackHandle track);
    
    /**
     * @brief Start playing an audio track
     * 
     * All layers in the track will begin playing.
     * 
     * @param track Handle to the track to play
     */
    void PlayTrack(TrackHandle track);
    
    /**
     * @brief Stop playing an audio track
     * 
     * All layers in the track will stop playing.
     * 
     * @param track Handle to the track to stop
     */
    void StopTrack(TrackHandle track);
    
    ///@}
    
    ///@name Layer Operations
    ///@{
    
    /**
     * @brief Add an audio layer to a track
     * 
     * Layers are individual sounds that play simultaneously within a track.
     * They can be controlled individually for volume and transitions.
     * 
     * @param track Handle to the track
     * @param layerName Name identifier for the layer
     * @param filepath Path to the audio file
     * @param group Optional name of the group this layer should belong to
     */
    void AddLayer(TrackHandle track, const string& layerName, const string& filepath, const string& group = "");
    
    /**
     * @brief Remove a layer from a track
     * 
     * @param track Handle to the track
     * @param layerName Name of the layer to remove
     */
    void RemoveLayer(TrackHandle track, const string& layerName);
    
    /**
     * @brief Set the volume of a specific layer
     * 
     * @param track Handle to the track
     * @param layerName Name of the layer
     * @param volume Volume level (0.0 to 1.0)
     */
    void SetLayerVolume(TrackHandle track, const string& layerName, float volume);
    
    /**
     * @brief Fade a layer's volume to a target value over time
     * 
     * @param track Handle to the track
     * @param layerName Name of the layer
     * @param targetVolume Target volume level (0.0 to 1.0)
     * @param duration Duration of the fade in milliseconds
     */
    void FadeLayer(TrackHandle track, const string& layerName, float targetVolume, std::chrono::milliseconds duration);
    ///@}
    
    ///@name Group Operations
    ///@{
    
    /**
     * @brief Create a new audio group
     * 
     * Audio groups allow for collective control of multiple sounds.
     * 
     * @param name Optional name for the group
     * @return GroupHandle Handle to the newly created group
     */
    GroupHandle CreateGroup(const string& name = "");
    
    /**
     * @brief Destroy an audio group
     * 
     * @param group Handle to the group to destroy
     */
    void DestroyGroup(GroupHandle group);
    
    /**
     * @brief Set the volume for an entire audio group
     * 
     * This affects all sounds assigned to the group.
     * 
     * @param group Handle to the group
     * @param volume Volume level (0.0 to 1.0)
     */
    void SetGroupVolume(GroupHandle group, float volume);
    
    /**
     * @brief Get the current volume for an audio group
     * 
     * @param group Handle to the group
     * @return float Current volume level (0.0 to 1.0)
     */
    float GetGroupVolume(GroupHandle group) const;
    
    /**
     * @brief Fade a group's volume to a target value over time
     * 
     * @param group Handle to the group
     * @param targetVolume Target volume level (0.0 to 1.0)
     * @param duration Duration of the fade in milliseconds
     */
    void FadeGroup(GroupHandle group, float targetVolume, std::chrono::milliseconds duration);
    ///@}
    
    ///@name Sound Operations
    ///@{
    
    /**
     * @brief Load a sound from a file
     * 
     * @param filepath Path to the audio file
     * @return SoundHandle Handle to the loaded sound
     */
    SoundHandle LoadSound(const string& filepath);
    
    /**
     * @brief Load a sound from a file and assign it to a group
     * 
     * @param filepath Path to the audio file
     * @param group Handle to the audio group to assign the sound to
     * @return SoundHandle Handle to the loaded sound
     */
    SoundHandle LoadSound(const string& filepath, GroupHandle group);
    
    /**
     * @brief Unload a previously loaded sound
     * 
     * @param sound Handle to the sound to unload
     */
    void UnloadSound(SoundHandle sound);
    
    /**
     * @brief Start playing a sound
     * 
     * This function was renamed from PlaySound to avoid conflicts
     * with the Windows PlaySound macro.
     * 
     * @param sound Handle to the sound to play
     */
    void StartSound(SoundHandle sound);
    
    /**
     * @brief Stop a currently playing sound
     * 
     * @param sound Handle to the sound to stop
     */
    void StopSound(SoundHandle sound);
    
    /**
     * @brief Set the volume of a sound
     * 
     * @param sound Handle to the sound
     * @param volume Volume level (0.0 to 1.0)
     */
    void SetSoundVolume(SoundHandle sound, float volume);
    
    /**
     * @brief Set the pitch of a sound for its next playback
     * 
     * @param sound Handle to the sound
     * @param pitch Pitch multiplier (1.0 = normal, 0.5 = half speed, 2.0 = double speed)
     */
    void SetSoundPitch(SoundHandle sound, float pitch);
    
    /**
     * @brief Check if a sound is currently playing
     * 
     * @param sound Handle to the sound
     * @return bool True if the sound is playing, false otherwise
     */
    bool IsSoundPlaying(SoundHandle sound) const;
    
    /**
     * @brief Play a random sound from a folder
     * 
     * Loads all .wav files from the specified folder and plays one randomly.
     * Sounds are cached after first load for efficiency.
     * 
     * @param folderPath Path to the folder containing sound files
     * @param group Optional group handle to assign the sounds to
     */
    void PlayRandomSoundFromFolder(const string& folderPath, GroupHandle group = 0);
    ///@}

private:
    /**
     * @brief Constructor - private due to singleton pattern
     */
    AudioManager();
    
    /**
     * @brief Destructor
     */
    ~AudioManager();
    
    // Disable copy and assignment
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    ///@name Internal Handle Management
    ///@{
    
    /**
     * @brief Generate a new track handle
     * @return TrackHandle New unique handle
     */
    TrackHandle NextTrackHandle() { return next_track_handle_++; }
    
    /**
     * @brief Generate a new group handle
     * @return GroupHandle New unique handle
     */
    GroupHandle NextGroupHandle() { return next_group_handle_++; }
    
    /**
     * @brief Generate a new sound handle
     * @return SoundHandle New unique handle
     */
    SoundHandle NextSoundHandle() { return next_sound_handle_++; }
    ///@}

    ///@name Resource Storage
    ///@{
    unique_ptr<AudioSystem> audio_system_;   ///< Core audio system
    unordered_map<TrackHandle, unique_ptr<AudioTrack>> tracks_;  ///< Track storage
    unordered_map<GroupHandle, unique_ptr<AudioGroup>> groups_;  ///< Group storage
    unordered_map<SoundHandle, unique_ptr<Sound>> sounds_;       ///< Sound storage
    unordered_map<string, GroupHandle> group_names_;             ///< Name-to-handle mapping for groups
    unordered_map<string, std::vector<SoundHandle>> folder_sounds_; ///< Cached sounds per folder
    ///@}

    ///@name Threading
    ///@{
    atomic<bool> running_{false};     ///< Flag indicating if audio system is running
    thread update_thread_;            ///< Thread for audio updates
    mutable mutex resource_mutex_;    ///< Mutex for thread-safe resource access (mutable for const methods)
    ///@}

    ///@name Handle Counters
    ///@{
    atomic<TrackHandle> next_track_handle_{1};    ///< Counter for generating unique track handles
    atomic<GroupHandle> next_group_handle_{1};    ///< Counter for generating unique group handles
    atomic<SoundHandle> next_sound_handle_{1};    ///< Counter for generating unique sound handles
    ///@}
};

} // namespace audio