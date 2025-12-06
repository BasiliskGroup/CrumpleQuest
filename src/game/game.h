#ifndef GAME_H
#define GAME_H

#include "util/includes.h"
#include "character/player.h"
#include "character/enemy.h"
#include "character/boss.h"
#include "levels/paper.h"
#include "audio/audio_manager.h"
#include "audio/sfx_player.h"
#include "ui/menu_manager.h"
#include "resource/animator.h"
#include "resource/animation.h"
#include "game/paperView.h"
#include <memory>

class Floor;
class UIElement;
class Animator;

class Game {
private:
    Engine* engine;

    std::unordered_map<std::string, Image*> images;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, Animation*> animations;
    std::unordered_map<std::string, Mesh*> meshes;

    Player* player;
    Floor* floor;
    Boss* boss;  // Exactly one boss instance
    std::string currentBiome = "notebook";  // Track current biome for floor alternation

    // rendering paper
    SingleSide* currentSide;
    Paper* paper;
    PaperView* paperView;

    // menu scene (separate from game scene)
    Scene2D* menuScene;
    StaticCamera2D* menuCamera;

    // track inputs
    bool rightWasDown = false;
    vec2 rightStartDown = vec2();
    bool foldIsActive = false;  // Track if a fold was activated

    bool kWasDown = false;
    bool escapeWasDown = false;
    bool rWasDown = false;
    bool upArrowWasDown = false;
    bool downArrowWasDown = false;
    bool leftArrowWasDown = false;
    bool rightArrowWasDown = false;

    // Mouse tracking for velocity-based sounds
    vec2 lastFoldMousePos;
    float lastFoldSoundTime = 0.0f;
    float elapsedTime = 0.0f;

    // menu management
    bool pendingReturnToMainMenu = false;
    bool pendingStartGame = false;
    bool pendingResetFloor = false;

    // audio
    audio::AudioManager& audioManager;
    audio::GroupHandle musicGroup;
    audio::GroupHandle sfxGroup;

    // player animator
    Animator* playerAnimator;

    // enemy data
    float pathTimer = 0;
    float maxPathTimer = 0.2;
    
    // boss visibility
    bool showBoss = true;

    // boss health bar
    Node2D* bossHealthBarBackground = nullptr;
    Node2D* bossHealthBarForeground = nullptr;
    vec2 healthBarPosition = vec2(0.0f, -4.1f);  // Position at bottom, between paper and screen edge
    float healthBarWidth = 8.0f;
    float healthBarHeight = 0.5f;

    // TODO maybe nove these to ui scenes
    std::vector<UIElement*> uiElements;

public:
    Game();
    ~Game();

    void addImage(std::string name, Image* image)          { this->images[name] = image; engine->getResourceServer()->getTextureServer()->add(image); }
    void addMaterial(std::string name, Material* material) { this->materials[name] = material; }
    void addAnimation(std::string name, std::string folder, unsigned int nImages);
    void addMesh(std::string name, Mesh* mesh)             { this->meshes[name] = mesh; }
    void addCollider(std::string name, Collider* collider) { this->currentSide->colliders[name] = collider; }

    // TODO temporary debug
    void addEnemy(Enemy* enemy) { this->currentSide->addEnemy(enemy); }
    void addUI(UIElement* uiElement) { this->uiElements.push_back(uiElement); }
    
    // getters
    Image* getImage(std::string name)       { return images[name]; }
    Material* getMaterial(std::string name) { return materials[name]; }
    Animation* getAnimation(std::string name) { return animations[name]; }
    Mesh* getMesh(std::string name)         { return meshes[name]; }
    Collider* getCollider(std::string name) { return currentSide->colliders[name]; }
    audio::AudioManager& getAudio()         { return audioManager; }
    audio::GroupHandle getMusicGroup()      { return musicGroup; }
    audio::GroupHandle getSFXGroup()        { return sfxGroup; }

    Engine*& getEngine() { return engine; }
    Scene2D* getScene() { return currentSide->getScene(); }
    Scene2D* getMenuScene() { return menuScene; }
    Paper* getPaper() { return paper; }
    SingleSide*& getSide() { return currentSide; }
    Player* getPlayer() { return player; }
    Floor* getFloor() { return floor; }
    Boss* getBoss() { return boss; }
    bool getShowBoss() const { return showBoss; }
    void setShowBoss(bool show) { showBoss = show; }

    auto& getEnemies() { return currentSide->getEnemies(); }

    // setters
    void setPlayer(Player* player) { this->player = player; }
    void setPaper(std::string str);
    void setSideToPaperSide();

    // initialization
    void initPaperView();
    void initMenus();
    void initBossHealthBar();
    void processReturnToMainMenu(); // Internal method to actually process the return

    // game flow
    void startGame();
    void returnToMainMenu();
    void setPendingStartGame(bool value) { pendingStartGame = value; }
    void switchToRoom(Paper* newPaper, int dx, int dy);
    void requestResetFloor(); // Request floor reset (deferred to next update)
    void resetFloor(); // Reset floor, boss, and floor-related state, keeping player

    // boss health bar
    void updateBossHealthBar();
    void showBossHealthBar(bool show);

    void update(float dt);
};

#endif