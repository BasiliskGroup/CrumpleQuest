#include "SoA/forceSoAs.h"
#include "util/print.h"


ManifoldSoA::ManifoldSoA(ForceSoA* forceSoA, uint capacity) : forceSoA(forceSoA) {
    this->capacity = capacity;

    // create all xtensors
    toDelete.resize(capacity);
    C0.resize(capacity); 
    rA.resize(capacity);
    rB.resize(capacity);
    normal.resize(capacity);
    friction.resize(capacity); 
    stick.resize(capacity); 
    simplex.resize(capacity); 
    forceIndex.resize(capacity); 

    // arrays for holding extra compute space
    tangent.resize(capacity);
    basis.resize(capacity);
    rAW.resize(capacity);
    rBW.resize(capacity);
}

/**
 * @brief Reserves enough space to the next power of 2 to insert numBodies rows. Assumes that the SoA is compact. This function should only be called by the ForceSoa
 * 
 * @param numBodies
 * @return the next free index in the SoA. 
 */
uint ManifoldSoA::reserve(uint numBodies) {
    // calculate next 2^n to hold all space
    uint neededSpace = pow(2, ceil(log2(size + numBodies)));
    
    if (neededSpace >= capacity) {
        resize(neededSpace);
    }

    // remove indices for reserveed elements
    for (uint i = size; i < size + numBodies; i++) {
        toDelete[i] = false;
    }

    uint nextFree = size;
    size += numBodies;
    return nextFree;
}

/**
 * @brief Resizes each tensor in the system up to the specified size. 
 * 
 * @param newCapacity new capacity of the tensor. If this is below the current size, the function is ignored. 
 */
void ManifoldSoA::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(size, newCapacity,
        toDelete, C0, rA, rB, normal, friction, stick, simplex, forceIndex, tangent, basis, rAW, rBW
    );

    // update capacity
    capacity = newCapacity;
}

void ManifoldSoA::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        C0, rA, rB, normal, friction, stick, simplex, forceIndex, tangent, basis, rAW, rBW
    );

    size = active;

    // TODO update foreign keys to forceSoA
    // reset values for toDelete since they were not mutated in the compact
    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
    }
}

void ManifoldSoA::warmstart() {
    // operate only on the first 'size' rows
    for (size_t i = 0; i < size; ++i) {
        tangent[i] = { -normal[i].y, normal[i].x };
        basis[i] = { normal[i], tangent[i] }; // TODO check this constructor

        
    }
}

void ManifoldSoA::remove(uint index) {
    toDelete[index] = true;
}