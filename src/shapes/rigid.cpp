#include "solver/physics.h"


Rigid::Rigid(Solver* solver, vec3 pos, vec2 scale, float density, float friction, vec3 vel, Mesh* mesh) : solver(solver), forces(nullptr), next(nullptr) {
    // Add to linked list
    next = solver->getBodies();
    solver->getBodies() = this;

    // compute intermediate values
    float volume = 1; // replace with mesh volume
    float mass = scale.x * scale.y * density * volume; 
    float moment = mass * glm::dot(scale, scale) / 12.0f; // TODO replace with mesh moment
    float radius = glm::length(scale * 0.5f);

    index = getBodySoA()->insert(this, pos, vel, scale, friction, mass, mesh->getIndex(), radius);
}   

Rigid::~Rigid() {
    // remove from solver linked list
    Rigid** p = &solver->getBodies();
    while (*p != this) {
        p = &(*p)->next;
    }
    *p = next;

    // does not delete forces, that is the soa's job. Only decouple
    Force* f = forces;
    Force* fnext;
    while(f != nullptr) {
        fnext = f->getNextA();

        f->markForDeletion();
        if (f->getBodyA() == this) {
            f->getBodyA() = nullptr;
            f->getNextA() = nullptr;
        }

        // move f along our list
        f = fnext;
    }

    // remove from SoA
    getBodySoA()->remove(index);
}

BodySoA* Rigid::getBodySoA() { 
    return solver->getBodySoA(); 
}

bool Rigid::constrainedTo(Rigid* other) const {
    // check if this body is constrained to the other body
    for (Force* f = forces; f != nullptr; f = f->getNext()) {
        // print("constrained search");
        // print(index);
        // print(f->getIndex());
        if (f->getBodyB() == other) {
            return true;
        }
    }
    return false;
}

void Rigid::draw() {

}