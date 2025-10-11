#include "SoA/forceSoAs.h"
#include "util/print.h"


ManifoldSoA::ManifoldSoA(ForceSoA* forceSoA, uint capacity) : forceSoA(forceSoA) {
    this->capacity = capacity;

    // create all xtensors
    toDelete   = xt::xtensor<bool, 1>::from_shape({capacity});
    C0         = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    rA         = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    rB         = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    normal     = xt::xtensor<float, 2>::from_shape({capacity, 2});
    friction   = xt::xtensor<float, 1>::from_shape({capacity});
    stick      = xt::xtensor<bool, 1>::from_shape({capacity});
    indexA     = xt::xtensor<uint, 2>::from_shape({capacity, 3});
    indexB     = xt::xtensor<uint, 2>::from_shape({capacity, 3});
    simplex    = xt::xtensor<float, 3>::from_shape({capacity, 3, 2});
    forceIndex = xt::xtensor<uint, 1>::from_shape({capacity});

    // arrays for holding extra compute space
    tangent = xt::xtensor<float, 2>::from_shape({capacity, 2});
    basis   = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    z       = xt::xtensor<float, 2>::from_shape({capacity, 2});
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
        toDelete(i) = false;
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
        toDelete, C0, rA, rB, normal, friction, stick, indexA, indexB, forceIndex, simplex, tangent, basis, z
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
        C0, rA, rB, normal, friction, stick, indexA, indexB, forceIndex, simplex, tangent, basis, z
    );

    size = active;

    // TODO update foreign keys to forceSoA
    // reset values for toDelete since they were not mutated in the compact
    for (uint i = 0; i < size; i++) {
        toDelete(i) = false;
    }
}

void ManifoldSoA::warmstart() {
    // operate only on the first 'size' rows
    for (size_t i = 0; i < size; ++i) {
        // tangent = [normal.y, -normal.x]
        tangent(i, 0) = normal(i, 1);
        tangent(i, 1) = -normal(i, 0);

        // basis = [[normal.x, normal.y], [tangent.x, tangent.y]]
        basis(i, 0, 0) = normal(i, 0);
        basis(i, 0, 1) = normal(i, 1);
        basis(i, 1, 0) = tangent(i, 0);
        basis(i, 1, 1) = tangent(i, 1);
    }
}

void ManifoldSoA::remove(uint index) {
    toDelete(index) = true;
}