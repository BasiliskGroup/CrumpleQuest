#include "solver/physics.h"

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB) 
: solver(solver), next(nullptr), bodyA(bodyA), bodyB(bodyB), nextA(nullptr) {
    // Add to solver linked list
    next = solver->getForces();
    solver->getForces() = this;

    // Add to body linked lists
    if (bodyA) {
        nextA = bodyA->getForces();
        bodyA->getForces() = this;
    }

    // TODO set default params
    // Probably flag SoA to set columns
}

Force::~Force()
{
    unlink();

    // remove self from SoA
    // TODO check if this should be done in bulk
    getForceSoA()->remove(index);
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
}

void Force::markForDeletion() {
    getForceSoA()->markForDeletion(index);
}

ushort Force::getType() {
    return getForceSoA()->getType()[index];
}

void Force::disable() {
    // TODO disable force
}

ForceSoA* Force::getForceSoA() { 
    return solver->getForceSoA(); 
}