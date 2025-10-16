#include "solver/physics.h"

Mesh::Mesh(Solver* solver, std::vector<vec2> verts) : solver(solver) {
    index = getMeshFlat()->insert(verts);
}

Mesh::~Mesh() {
    // remove self from meshFlat
    getMeshFlat()->remove(index);
}

MeshFlat* Mesh::getMeshFlat() {
    return solver->getMeshFlat();
}