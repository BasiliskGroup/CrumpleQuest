#ifndef RIGID_H
#define RIGID_H

#include "util/indexed.h"

class Solver;
class Force;
class Mesh;
class BodyTable;

class Rigid : public Indexed {
private:
    Solver* solver;
    Force* forces;
    Rigid* next;

    // used to cache relations on system graph
    std::vector<std::pair<uint, ushort>> relations;

public:
    Rigid(Solver* solver, vec3 pos, vec2 scale, float density, float friction, vec3 vel, Mesh* mesh);
    ~Rigid();

    // getters
    Rigid*& getNext()   { return next; }
    Force*& getForces() { return forces; }
    BodyTable* getBodyTable();

    // determines if two objects are constrained (no collision needed)
    void precomputeRelations();
    ushort constrainedTo(uint other) const;
    void draw();
};

#endif