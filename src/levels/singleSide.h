#ifndef SINGLE_SIDE_H
#define SINGLE_SIDE_H

#include "levels/navmesh.h"
#include "levels/obstacle.h"
#include "character/enemy.h"

class SingleSide {  
private:
    std::vector<Obstacle*> obstacles;
    Navmesh* navmesh;
    std::vector<Enemy> enemies;

public:
    SingleSide();
    SingleSide(const SingleSide& other);
    SingleSide(SingleSide&& other);
    ~SingleSide();

    SingleSide& operator=(const SingleSide& other) noexcept;
    SingleSide& operator=(SingleSide&& other) noexcept;

    void generateNavmesh();
    void update(float dt);

private:
    void clear();
};

#endif