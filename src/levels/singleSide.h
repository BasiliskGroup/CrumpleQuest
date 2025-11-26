#ifndef SINGLE_SIDE_H
#define SINGLE_SIDE_H

#include "levels/navmesh.h"

class Enemy;
class Game;
class DamageZone;

class SingleSide {  
public:
    static std::unordered_map<std::string, std::function<SingleSide*()>> templates;
    static void generateTemplates(Game* game);

private:
    Scene2D* scene;
    StaticCamera2D* camera;
    std::vector<Enemy*> enemies;
    std::vector<DamageZone*> damageZones;

public:
    SingleSide(Game* game);
    SingleSide(const SingleSide& other) noexcept;
    SingleSide(SingleSide&& other) noexcept;
    ~SingleSide();

    SingleSide& operator=(const SingleSide& other) noexcept;
    SingleSide& operator=(SingleSide&& other) noexcept;

    // getters
    Scene2D*& getScene() { return scene; }
    auto& getEnemies() { return enemies; }

    void addEnemy(Enemy* enemy) { this->enemies.push_back(enemy); }

    void generateNavmesh();
    void update(const vec2& playerPos, float dt);

    void addDamageZone(DamageZone* zone) { this->damageZones.push_back(zone); }

private:
    void clear();
};

#endif