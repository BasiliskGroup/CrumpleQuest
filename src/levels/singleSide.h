#ifndef SINGLE_SIDE_H
#define SINGLE_SIDE_H

#include "levels/navmesh.h"
#include "character/enemy.h"

class Game;

class SingleSide {  
public:
    static std::unordered_map<std::string, std::function<SingleSide*()>> templates;
    static void generateTemplates(Game* game);

private:
    Scene2D* scene;
    StaticCamera2D* camera;
    std::vector<Enemy*> enemies;

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
    void update(float dt);

private:
    void clear();
};

#endif