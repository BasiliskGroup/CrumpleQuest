#include "solver/physics.h"

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB) 
: solver(solver), next(nullptr), bodyA(bodyA), bodyB(bodyB), nextA(nullptr), twin(nullptr), prev(nullptr), prevA(nullptr) {
    solver->insert(this);
    bodyA->insert(this);

    // TODO set default params
    // Probably flag Table to set columns
}

Force::~Force()
{
    markAsDeleted(); // clean up in table
    solver->remove(this);
    bodyA->remove(this);

    // delete the twin if it has not already been deleted, force twins must exist in pairs
    if (twin != nullptr) {

        // need to mark twin->this as nullptr so twin doesn't call this delete
        twin->twin = nullptr;
        delete twin;
    }

    // clean up pointers
    // TODO check if these are alraedy cleared by remove functions
    bodyA = nullptr;
    bodyB = nullptr;
    solver = nullptr;
}

void Force::markAsDeleted() {
    solver->getForceTable()->markAsDeleted(index);
}

ushort Force::getType() {
    return solver->getForceTable()->getType()[index];
}

void Force::disable() {
    // TODO disable force
}