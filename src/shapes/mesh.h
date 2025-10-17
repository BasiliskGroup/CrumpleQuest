#ifndef MESH_H
#define MESH_H

#include "util/includes.h"

class Solver;
class MeshFlat;

class Mesh {
private:
    Solver* solver;

    // for Table access
    uint index;

public:
    Mesh(Solver* solver, std::vector<vec2> verts);
    ~Mesh();

    uint getIndex() { return index; }
    void setIndex(uint index) { this->index = index; }

    MeshFlat* getMeshFlat();
};

#endif