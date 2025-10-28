#include "solver/physics.h"

Collider::Collider(Solver* solver, std::vector<vec2> verts) : solver(solver) {
    index = getColliderFlat()->insert(verts);
}

Collider::~Collider() {
    // remove self from ColliderFlat
    getColliderFlat()->remove(index);
}

ColliderFlat* Collider::getColliderFlat() {
    return solver->getColliderFlat();
}