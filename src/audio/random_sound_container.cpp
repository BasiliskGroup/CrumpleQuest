#include "audio/random_sound_container.h"
#include "audio/audio_manager.h"
#include <iostream>
#include <random>
#include <filesystem>
#include "miniaudio/miniaudio.h"

namespace fs = std::filesystem;

namespace audio {

RandomSoundContainer::RandomSoundContainer(const std::string& name, const RandomSoundContainerConfig& config)
    : name_(name)
    , config_(config)
    , last_played_(0)
    , rng_(std::random_device{}())
{
}

RandomSoundContainer::~RandomSoundContainer() {
    // Unload all sounds
    AudioManager& audio = AudioManager::GetInstance();
    for (SoundHandle sound : sounds_) {
        audio.UnloadSound(sound);
    }
}

void RandomSoundContainer::AddSound(const std::string& filepath) {
    AudioManager& audio = AudioManager::GetInstance();
    SoundHandle handle;
    
    // Load sound with group if specified in config
    if (config_.group != 0) {
        handle = audio.LoadSound(filepath, config_.group);
    } else {
        handle = audio.LoadSound(filepath);
    }
    
    if (handle != 0) {
        sounds_.push_back(handle);
    }
}

void RandomSoundContainer::LoadFromFolder(const std::string& folderPath) {
    AudioManager& audio = AudioManager::GetInstance();

    fs::path dir(folderPath);
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        return;
    }

    int count = 0;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file())
            continue;

        auto path = entry.path();
        if (path.extension() == ".wav") {
            // Check duration if maxDuration is set
            if (config_.maxDuration > 0.0f) {
                ma_decoder decoder;
                ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 0, 0);
                
                if (ma_decoder_init_file(path.string().c_str(), &decoderConfig, &decoder) == MA_SUCCESS) {
                    ma_uint64 lengthInFrames;
                    ma_decoder_get_length_in_pcm_frames(&decoder, &lengthInFrames);
                    float durationInSeconds = (float)lengthInFrames / (float)decoder.outputSampleRate;
                    ma_decoder_uninit(&decoder);
                    
                    if (durationInSeconds > config_.maxDuration) {
                        continue; // Skip this file
                    }
                }
            }
            
            AddSound(path.string());
            count++;
        }
    }
}

void RandomSoundContainer::Play() {
    PlayWithVolume(1.0f); // Default full volume
}

void RandomSoundContainer::PlayWithVolume(float volume) {
    if (sounds_.empty()) {
        return;
    }
    
    SoundHandle selected = SelectRandomSound();
    
    AudioManager& audio = AudioManager::GetInstance();
    
    // Apply pitch shift if range is set
    if (config_.pitchMin != 1.0f || config_.pitchMax != 1.0f) {
        std::uniform_real_distribution<float> pitchDist(config_.pitchMin, config_.pitchMax);
        float pitch = pitchDist(rng_);
        audio.SetSoundPitch(selected, pitch);
    }
    
    // Apply volume
    audio.SetSoundVolume(selected, volume);
    
    audio.StartSound(selected);
    last_played_ = selected;
}

void RandomSoundContainer::StopAll() {
    AudioManager& audio = AudioManager::GetInstance();
    for (SoundHandle sound : sounds_) {
        audio.StopSound(sound);
    }
}

void RandomSoundContainer::SetPitchRange(float minPitch, float maxPitch) {
    config_.pitchMin = minPitch;
    config_.pitchMax = maxPitch;
}

void RandomSoundContainer::SetAvoidRepeat(bool avoid) {
    config_.avoidRepeat = avoid;
}

SoundHandle RandomSoundContainer::SelectRandomSound() {
    if (sounds_.size() == 1) {
        return sounds_[0];
    }
    
    if (config_.avoidRepeat && sounds_.size() > 1 && last_played_ != 0) {
        // Create a list excluding the last played sound
        std::vector<SoundHandle> available;
        for (SoundHandle sound : sounds_) {
            if (sound != last_played_) {
                available.push_back(sound);
            }
        }
        
        std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
        return available[dist(rng_)];
    }
    
    // Random selection from all sounds
    std::uniform_int_distribution<size_t> dist(0, sounds_.size() - 1);
    return sounds_[dist(rng_)];
}

} // namespace audio
