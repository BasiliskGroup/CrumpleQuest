#include "tables/forceSoAs.h"
#include "util/print.h"


ForceSoA::ForceSoA(uint capacity) {
    resize(capacity);

    // create SoAs
    manifoldSoA = new ManifoldSoA(this, capacity);
}

ForceSoA::~ForceSoA() {
    delete manifoldSoA;
}

void ForceSoA::markForDeletion(uint index) { 
    toDelete[index] = true; 
    forces[index] = nullptr;
}

void ForceSoA::warmstart(float alpha, float gamma) {
    for (uint i = 0; i < size; i++) {
        for (uint j = 0; j < ROWS; j++) {
            lambda[i][j] *= alpha * gamma;
            penalty[i][j] = glm::clamp(penalty[i][j] * gamma, PENALTY_MIN, PENALTY_MAX);
            penalty[i][j] = glm::min(penalty[i][j], stiffness[i][j]);
        }
    }
}

void ForceSoA::reserveManifolds(uint numPairs, uint& forceIndex, uint& manifoldIndex) {
    manifoldIndex = manifoldSoA->reserve(numPairs);
    uint numBodies = 2 * numPairs;
    
    uint neededSpace = pow(2, ceil(log2(size + numBodies)));

    if (neededSpace >= capacity) {
        resize(neededSpace);
    }

    // ensure reserved slots arent deleted
    for (uint i = size; i < size + numBodies; i++) {
        toDelete[i] = false;
    }

    forceIndex = size;
    size += numBodies;
}

void ForceSoA::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(size, newCapacity, 
        forces, toDelete, J, C, motor, stiffness, fracture, fmax, fmin, penalty, lambda, H, type, specialIndex, bodyIndex, isA
    );

    // NOTE we do not have to explicitly set our deletes to false since they outside of size

    // update capacity
    capacity = newCapacity;
}

void ForceSoA::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        // nothing to delete
        return;
    }

    // delete marked forces before we lose them in compact
    for(uint i = 0; i < size; i++) {
        if (toDelete[i] == true && forces[i] != nullptr) {
            delete forces[i];
        }
    }

    // todo write new compact function
    compactTensors(toDelete, size, 
        forces, J, C, motor, stiffness, fracture, fmax,fmin, penalty, lambda, H, type, specialIndex, bodyIndex, isA
    );

    size = active;

    // update maps
    for (uint i = 0; i < size; i++) {
        // update force object
        forces[i]->setIndex(i);

        // reset values for toDelete since they were not mutated in the compact
        toDelete[i] = false;
    }

    // compact special tables
    manifoldSoA->compact();
}

int ForceSoA::insert() {
    // Skipped as requested - you'll handle the special cases
    return 0;
}

void ForceSoA::remove(uint index) {
    toDelete[index] = true;
}