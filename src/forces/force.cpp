#include "solver/physics.h"

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB) 
: solver(solver), next(nullptr), bodyA(bodyA), bodyB(bodyB), nextA(nullptr), twin(nullptr) {
    // Add to solver linked list
    next = solver->getForces();
    solver->getForces() = this;

    // Add to body linked lists
    if (bodyA) {
        nextA = bodyA->getForces();
        bodyA->getForces() = this;
    }

    // TODO set default params
    // Probably flag Table to set columns
}

Force::~Force()
{
    unlink();

    // remove self from Table
    // TODO check if this should be done in bulk
    getForceTable()->remove(index);
}

void Force::unlink() {
    // Remove from solver linked list
    Force** p = &solver->getForces();
    while (*p != this) {
        p = &(*p)->next;
    }
    *p = next;

    // Remove from body linked lists
    if (bodyA) {
        p = &bodyA->getForces();
        while (*p != this) {
            p = &(*p)->nextA;
        }
        if (*p == this) {
            *p = nextA;
        }
    }

    // remove self from twin and mark for deletion
    if (twin != nullptr) {
        twin->markForDeletion();
        twin->bodyB = nullptr;
        twin->twin = nullptr;
        twin = nullptr;
    }
}

void Force::markForDeletion() {
    getForceTable()->markForDeletion(index);
}

ushort Force::getType() {
    return getForceTable()->getType()[index];
}

void Force::disable() {
    // TODO disable force
}

ForceTable* Force::getForceTable() { 
    return solver->getForceTable(); 
}