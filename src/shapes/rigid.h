#ifndef RIGID_H
#define RIGID_H

#include "util/indexed.h"

class Solver;
class Force;
class Mesh;
class BodySoA;

class Rigid : public Indexed {
private:
    Solver* solver;
    Force* forces;
    Rigid* next;

public:
    Rigid(Solver* solver, vec3 pos, vec2 scale, float density, float friction, vec3 vel, Mesh* mesh);
    ~Rigid();

    // getters
    Rigid*& getNext()   { return next; }
    Force*& getForces() { return forces; }
    BodySoA* getBodySoA();

    // determines if two objects are constrained (no collision needed)
    bool constrainedTo(Rigid* other) const;
    void draw();
};

#endif