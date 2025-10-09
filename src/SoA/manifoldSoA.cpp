#include "SoA/forceSoAs.h"
#include "util/print.h"


ManifoldSoA::ManifoldSoA(ForceSoA* forceSoA, uint capacity) : forceSoA(forceSoA) {
    this->capacity = capacity;

    // create all xtensors
    toDelete   = xt::xtensor<bool, 1>::from_shape({capacity});
    C0         = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    rA         = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    rB         = xt::xtensor<float, 3>::from_shape({capacity, 2, 2});
    normal     = xt::xtensor<float, 2>::from_shape({capacity, 3});
    friction   = xt::xtensor<float, 1>::from_shape({capacity});
    stick      = xt::xtensor<bool, 1>::from_shape({capacity});
    indexA     = xt::xtensor<uint, 2>::from_shape({capacity, 3});
    indexB     = xt::xtensor<uint, 2>::from_shape({capacity, 3});
    simplex    = xt::xtensor<float, 3>::from_shape({capacity, 3, 2});
    forceIndex = xt::xtensor<uint, 1>::from_shape({capacity});
}

/**
 * @brief Reserves enough space to the next power of 2 to insert numPairs rows. Assumes that the SoA is compact. This function should only be called by the ForceSoa
 * 
 * @param numPairs
 * @return the next free index in the SoA. 
 */
uint ManifoldSoA::reserve(uint numPairs) {
    // calculate next 2^n to hold all space
    uint neededSpace = pow(2, ceil(log2(size + numPairs)));
    
    if (neededSpace >= capacity) {
        resize(neededSpace);
    }

    // remove indices for reserveed elements
    for (uint i = size; i < size + numPairs; i++) {
        toDelete(i) = false;
    }

    uint nextFree = size;
    size += numPairs;
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
        toDelete, C0, rA, rB, normal, friction, stick, indexA, indexB, forceIndex, simplex
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
        C0, rA, rB, normal, friction, stick,
        indexA, indexB, forceIndex, simplex
    );

    size = active;

    // TODO update foreign keys to forceSoA
    // reset values for toDelete since they were not mutated in the compact
    for (uint i = 0; i < size; i++) {
        toDelete(i) = false;
    }
}

void ManifoldSoA::insert() {

}

void ManifoldSoA::remove(uint index) {
    toDelete(index) = true;
}