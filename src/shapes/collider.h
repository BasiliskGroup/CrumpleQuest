#ifndef COLLIDER_H
#define COLLIDER_H

#include "util/includes.h"

class Solver;
class ColliderFlat;

class Collider {
private:
    Solver* solver;

    // for Table access
    uint index;

public:
    Collider(Solver* solver, std::vector<vec2> verts);
    ~Collider();

    uint getIndex() { return index; }
    void setIndex(uint index) { this->index = index; }

    ColliderFlat* getColliderFlat();
};

#endif