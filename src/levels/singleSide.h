#ifndef SINGLE_SIDE_H
#define SINGLE_SIDE_H

#include "levels/navmesh.h"
#include "character/enemy.h"

class Game;

class SingleSide {  
public:
    static std::unordered_map<std::string, SingleSide> templates;

private:
    std::vector<Node2D*> obstacles;
    Navmesh* navmesh;
    std::vector<Enemy> enemies;

public:
    SingleSide();
    SingleSide(const SingleSide& other) noexcept;
    SingleSide(SingleSide&& other) noexcept;
    ~SingleSide();

    SingleSide& operator=(const SingleSide& other) noexcept;
    SingleSide& operator=(SingleSide&& other) noexcept;

    void addObstacle(Node2D* obstacle) { this->obstacles.push_back(obstacle); }
    void addEnemy(Enemy enemy) { this->enemies.push_back(enemy); }

    void generateNavmesh();
    void update(float dt);

    static void generateTemplates(Game* game);

private:
    void clear();
};

#endif