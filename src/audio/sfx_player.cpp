#include "audio/sfx_player.h"
#include <iostream>

namespace audio {

SFXPlayer::SFXPlayer() 
    : sfx_group_(0), initialized_(false) {
}

void SFXPlayer::Initialize(GroupHandle sfx_group) {
    if (initialized_) {
        return; // Already initialized
    }
    
    sfx_group_ = sfx_group;
    initialized_ = true;
    
    // Load fold start sounds
    RandomSoundContainerConfig foldConfig;
    foldConfig.avoidRepeat = true;
    foldConfig.pitchMin = 0.9f;
    foldConfig.pitchMax = 1.1f;
    LoadCollection("fold", "sounds/sfx/paper/move_sounds", foldConfig);
    
    // Load fold end sounds
    RandomSoundContainerConfig foldEndConfig;
    foldEndConfig.avoidRepeat = true;
    foldEndConfig.pitchMin = 0.9f;
    foldEndConfig.pitchMax = 1.1f;
    LoadCollection("fold_end", "sounds/sfx/paper/move_sounds", foldEndConfig);
    
    // Load flip sounds
    RandomSoundContainerConfig flipConfig;
    flipConfig.avoidRepeat = true;
    flipConfig.pitchMin = 0.9f;
    flipConfig.pitchMax = 1.1f;
    LoadCollection("flip", "sounds/sfx/paper/flip_sounds", flipConfig);
    
    // Load menu touch sounds
    RandomSoundContainerConfig menuTouchConfig;
    menuTouchConfig.avoidRepeat = true;
    menuTouchConfig.pitchMin = 0.8f;
    menuTouchConfig.pitchMax = 1.2f;
    LoadCollection("menu_touch", "sounds/sfx/paper/touch_sounds", menuTouchConfig);
    
    // Load player woosh/attack sounds
    RandomSoundContainerConfig wooshConfig;
    wooshConfig.avoidRepeat = true;
    wooshConfig.pitchMin = 0.9f;
    wooshConfig.pitchMax = 1.1f;
    LoadCollection("woosh", "sounds/sfx/player/woosh-2", wooshConfig);
    
    // Load hit sounds
    RandomSoundContainerConfig clipflyHitConfig;
    clipflyHitConfig.avoidRepeat = true;
    clipflyHitConfig.pitchMin = 1.0f;
    clipflyHitConfig.pitchMax = 1.2f;
    LoadCollection("hit-clipfly", "sounds/sfx/enemy/hit-metal-ring", clipflyHitConfig);

    // Load hit sounds
    RandomSoundContainerConfig stapleRemoverHitConfig;
    stapleRemoverHitConfig.avoidRepeat = true;
    stapleRemoverHitConfig.pitchMin = 0.8f;
    stapleRemoverHitConfig.pitchMax = 1.0f;
    LoadCollection("hit-staple-remover", "sounds/sfx/enemy/hit-metal-ring", stapleRemoverHitConfig);
    
    // Load hit sounds
    RandomSoundContainerConfig glueHitConfig;
    glueHitConfig.avoidRepeat = true;
    glueHitConfig.pitchMin = 0.9f;
    glueHitConfig.pitchMax = 1.1f;
    LoadCollection("hit-glue", "sounds/sfx/enemy/hit-wood", glueHitConfig);
    
    // Load player hit sounds
    RandomSoundContainerConfig playerHitConfig;
    playerHitConfig.avoidRepeat = true;
    playerHitConfig.pitchMin = 0.7f;
    playerHitConfig.pitchMax = 0.9f;
    LoadCollection("hit-player", "sounds/sfx/enemy/hit-metal-ring", playerHitConfig);
}

void SFXPlayer::LoadCollection(const std::string& name, 
                              const std::string& folder_path,
                              const RandomSoundContainerConfig& config) {
    // Create a new config with the SFX group
    RandomSoundContainerConfig sfx_config = config;
    sfx_config.group = sfx_group_;
    
    auto container = std::make_unique<RandomSoundContainer>(name, sfx_config);
    container->LoadFromFolder(folder_path);
    
    containers_[name] = std::move(container);
}

void SFXPlayer::Play(const std::string& name) {
    if (!initialized_) {
        std::cerr << "SFXPlayer: Not initialized! Call Initialize() first." << std::endl;
        return;
    }
    
    auto it = containers_.find(name);
    if (it != containers_.end()) {
        it->second->Play();
    } else {
        std::cerr << "SFXPlayer: Sound effect '" << name << "' not found" << std::endl;
    }
}

SFXPlayer& SFXPlayer::Get() {
    static SFXPlayer instance; // Meyers singleton - thread-safe in C++11+
    return instance;
}

} // namespace audio
