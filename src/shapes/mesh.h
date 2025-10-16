#ifndef MESH_H
#define MESH_H

#include "util/indexed.h"

class Solver;
class MeshFlat;

class Mesh : public Indexed {
private:
    Solver* solver;

public:
    Mesh(Solver* solver, std::vector<vec2> verts);
    ~Mesh();

    MeshFlat* getMeshFlat();
};

#endif