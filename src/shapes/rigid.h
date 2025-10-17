#ifndef RIGID_H
#define RIGID_H

class Solver;
class Force;
class Mesh;
class BodyTable;

class Rigid {
private:
    Solver* solver;
    Force* forces;
    Rigid* next; // next in solver list
    Rigid* prev; // prev in solver list

    // used to cache relations on system graph
    std::vector<std::pair<uint, ushort>> relations;

    // for Table access
    uint index;

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

    uint getIndex() { return index; }
    void setIndex(uint index) { this->index = index; }

    // determines if two objects are constrained (no collision needed)
    void precomputeRelations();
    ushort constrainedTo(uint other) const;
    void draw();
};

#endif