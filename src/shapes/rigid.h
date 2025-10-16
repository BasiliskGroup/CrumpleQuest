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
    Rigid* next; // next in solver list
    Rigid* prev; // prev in solver list

    // used to cache relations on system graph
    std::vector<std::pair<uint, ushort>> relations;

public:
    Rigid(Solver* solver, vec3 pos, vec2 scale, float density, float friction, vec3 vel, Mesh* mesh);
    ~Rigid();

    // list management
    void insert(Force* force);
    void remove(Force* force);

    // getters
    Solver*& getSolver() { return solver; }
    Rigid*& getNext()    { return next; }
    Force*& getForces()  { return forces; }
    Rigid*& getPrev()    { return prev; }

    // determines if two objects are constrained (no collision needed)
    void precomputeRelations();
    ushort constrainedTo(uint other) const;
    void draw();
};

#endif