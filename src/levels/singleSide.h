#ifndef SINGLE_SIDE_H
#define SINGLE_SIDE_H

#include "util/includes.h"

class Enemy;
class Game;
class DamageZone;
class Player;

class SingleSide {  
public:
    static std::unordered_map<std::string, std::function<SingleSide*()>> templates;
    static Node2D* genPlayerNode(Game* game, SingleSide* side);
    static void generateTemplates(Game* game);

    std::unordered_map<std::string, Collider*> colliders;

private:
    Scene2D* scene;
    StaticCamera2D* camera;
    std::vector<Enemy*> enemies;
    std::vector<DamageZone*> damageZones;

    Node2D* background;
    Node2D* playerNode;
    Node2D* weaponNode;

    // point to nodes in scene so no need to delete
    std::vector<Node2D*> walls; 

public:
    SingleSide(Game* game, std::string mesh, std::string material);
    SingleSide(const SingleSide& other) noexcept;
    SingleSide(SingleSide&& other) noexcept;
    ~SingleSide();

    SingleSide& operator=(const SingleSide& other) noexcept;
    SingleSide& operator=(SingleSide&& other) noexcept;

    // getters
    Scene2D* getScene() { return scene; }
    auto& getEnemies() { return enemies; }
    Collider* getCollider(std::string name) { return colliders[name]; }
    Node2D* getBackground() { return background; }
    Node2D* getPlayerNode() { return playerNode; }
    Node2D* getWeaponNode() { return weaponNode; }

    void addEnemy(Enemy* enemy) { this->enemies.push_back(enemy); }
    void addWall(Node2D* wall) { this->walls.push_back(wall); }
    void addDamageZone(DamageZone* zone) { this->damageZones.push_back(zone); }
    void addCollider(std::string name, Collider* collider) { this->colliders[name] = collider; }

    void generateNavmesh();
    void update(const vec2& playerPos, float dt, Player* player = nullptr);
    void clearWalls();
    void loadResources();
    void adoptEnemy(Enemy* enemy, SingleSide* fromSide);

private:
    void clear();
};

#endif