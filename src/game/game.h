#ifndef GAME_H
#define GAME_H

#include "util/includes.h"
#include "character/player.h"
#include "character/enemy.h"
#include "levels/paper.h"
#include "audio/audio_manager.h"
#include "ui/menu_manager.h"

class Floor;
class UIElement;

class Game {
private:
    Engine* engine;

    std::unordered_map<std::string, Image*> images;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, Mesh*> meshes;

    Player* player;
    Floor* floor;

    // rendering paper
    SingleSide* currentSide;
    Paper* paper;

    // track inputs
    bool leftWasDown = false;
    vec2 LeftStartDown = vec2();

    bool kWasDown = false;

    // audio
    audio::AudioManager& audioManager;

    // menu manager
    MenuManager* menuManager;

    // TODO maybe nove these to ui scenes
    std::vector<UIElement*> uiElements;

public:
    Game();
    ~Game();

    void addImage(std::string name, Image* image)          { this->images[name] = image; }
    void addMaterial(std::string name, Material* material) { this->materials[name] = material; }
    void addMesh(std::string name, Mesh* mesh)             { this->meshes[name] = mesh; }
    void addCollider(std::string name, Collider* collider) { this->currentSide->colliders[name] = collider; }

    // TODO temporary debug
    void addEnemy(Enemy* enemy) { this->currentSide->addEnemy(enemy); }
    void addUI(UIElement* uiElement) { this->uiElements.push_back(uiElement); }
    
    // getters
    Image* getImage(std::string name)       { return images[name]; }
    Material* getMaterial(std::string name) { return materials[name]; }
    Mesh* getMesh(std::string name)         { return meshes[name]; }
    Collider* getCollider(std::string name) { return currentSide->colliders[name]; }
    audio::AudioManager& getAudio()         { return audioManager; }
    MenuManager* getMenus()                 { return menuManager; }

    Engine*& getEngine() { return engine; }
    Scene2D* getScene() { return currentSide->getScene(); }
    Paper* getPaper() { return paper; }
    SingleSide*& getSide() { return currentSide; }

    auto& getEnemies() { return currentSide->getEnemies(); }

    // setters
    void setPlayer(Player* player) { this->player = player; }
    void setPaper(std::string str);

    // initialization
    void initMenus();

    void update(float dt);
};

#endif