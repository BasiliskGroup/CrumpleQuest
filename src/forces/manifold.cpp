#include "physics.h"

Manifold::Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB, uint index) : Force(solver, bodyA, bodyB) {
    this->index = index;
}

Manifold::~Manifold() {

}

void Manifold::draw() const {

}