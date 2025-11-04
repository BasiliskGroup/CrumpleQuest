#ifndef AI_H
#define AI_H

#include "levels/navmesh.h"

class AI {
    Navmesh* navmesh;
    std::vector<vec2> path;

public:
    AI(Navmesh* navmesh);
    ~AI() = default;

    Navmesh*& getNavmesh() { return this->navmesh; }
    void setNavmesh(Navmesh* navmesh) { this->navmesh = navmesh; }

    vec2 targetPosition(const vec2& start);
    vec2 targetDirection(const vec2& start);
    void updatePath(const vec2& start, const vec2& dest);
};

#endif