#ifndef GAME_H
#define GAME_H

#include "util/includes.h"
#include "character/player.h"
#include "character/enemy.h"
#include "levels/paper.h"

class Floor;

class Game {
private:
    Engine* engine;
    Scene2D* scene;
    Scene2D* voidScene;
    StaticCamera2D* camera;

    std::unordered_map<std::string, Collider*> colliders;
    std::unordered_map<std::string, Image*> images;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, Mesh*> meshes;

    Player* player;
    Floor* floor;
    std::vector<Enemy*> enemies;

    // rendering paper
    Paper* paper;
    Node2D* paperNode;

    // track mouse inputs
    bool leftWasDown = false;
    vec2 LeftStartDown = vec2();

public:
    Game();
    ~Game();

    void addImage(std::string name, Image* image)          { this->images[name] = image; }
    void addMaterial(std::string name, Material* material) { this->materials[name] = material; }
    void addMesh(std::string name, Mesh* mesh)             { this->meshes[name] = mesh; }
    void addCollider(std::string name, Collider* collider) { this->colliders[name] = collider; }

    void addEnemy(Enemy* enemy) { this->enemies.push_back(enemy); }
    
    // getters
    Image* getImage(std::string name)       { return images[name]; }
    Material* getMaterial(std::string name) { return materials[name]; }
    Mesh* getMesh(std::string name)         { return meshes[name]; }
    Collider* getCollider(std::string name) { return colliders[name]; }

    Engine*& getEngine() { return engine; }
    Scene2D*& getScene() { return scene; }
    Scene2D* getVoidScene() { return voidScene; }
    Paper* getPaper() { return paper; }

    // setters
    void setPlayer(Player* player) { this->player = player; }
    void setPaper(Paper* paper) { this->paper = paper; }

    void update(float dt);
};

#endif