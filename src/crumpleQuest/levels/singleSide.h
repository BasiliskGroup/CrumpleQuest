#ifndef SINGLE_SIDE_H
#define SINGLE_SIDE_H

#include "crumpleQuest/levels/navmesh.h"

class Enemy;

class SingleSide {  
private:
    Navmesh* navmesh;
    std::vector<Enemy> enemies;

public:
    SingleSide();
    SingleSide(const SingleSide& other);
    ~SingleSide();

    void update(float dt);

};

#endif