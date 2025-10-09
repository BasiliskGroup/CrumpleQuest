#include "solver/physics.h"

Mesh::Mesh(Solver* solver, std::vector<vec2> verts) : solver(solver) {
    index = getMeshSoA()->insert(verts);
}

Mesh::~Mesh() {
    // remove self from meshFlat
    getMeshSoA()->remove(index);
}

MeshSoA* Mesh::getMeshSoA() {
    return solver->getMeshSoA();
}