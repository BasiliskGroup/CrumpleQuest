#include "SoA/forceSoAs.h"
#include "util/print.h"


ManifoldSoA::ManifoldSoA(ForceSoA* forceSoA, uint capacity) : forceSoA(forceSoA) {
    this->capacity = capacity;

    // fill freeIndices
    for (uint i = 0; i < capacity; ++i) {
        freeIndices.insert(i);
    }

    // create all xtensors
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
 * @brief Reserves enough space to the next power of 2 to insert numPairs rows. Assumes that the SoA is compact. 
 * 
 * @param numPairs 
 * @return the next free index in the SoA. 
 */
uint ManifoldSoA::reserve(uint numPairs) {
    // calculate next 2^n to hold all space
    uint neededSpace = pow(2, ceil(log2(size + numPairs)));
    if (neededSpace < capacity) {
        return size;
    }

    resize(neededSpace);
    return size;
}

/**
 * @brief Resizes each tensor in the system up to the specified size. 
 * 
 * @param newCapacity new capacity of the tensor. If this is below the current size, the function is ignored. 
 */
void ManifoldSoA::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensor(C0,         size, newCapacity);
    expandTensor(rA,         size, newCapacity);
    expandTensor(rB,         size, newCapacity);
    expandTensor(normal,     size, newCapacity);
    expandTensor(friction,   size, newCapacity);
    expandTensor(stick,      size, newCapacity);
    expandTensor(indexA,     size, newCapacity);
    expandTensor(indexB,     size, newCapacity);
    expandTensor(forceIndex, size, newCapacity);
    expandTensor(simplex,    size, newCapacity);

    // add all new indices to freeIndices
    for (uint i = capacity; i < newCapacity; ++i) {
        freeIndices.insert(i);
    }

    // update capacity
    capacity = newCapacity;
}

void ManifoldSoA::compact() {
    if (freeIndices.empty()) return; // nothing to remove

    compactTensors(freeIndices, capacity, forceFKs,
        C0, rA, rB, normal, friction, stick,
        indexA, indexB, forceIndex, simplex
    );

    // go through and write all foreign keys back into ForceSoA
    auto special = forceSoA->getSpecial();
    for (const auto& pair : forceFKs) {
        // update force keys to match our rows
        special(pair.second) = pair.first;
    }

    // update freeIndices
    freeIndices.clear();
    for (uint i = size; i < capacity; i++) {
        freeIndices.insert(i);
    }
}

void ManifoldSoA::insert() {

}

void ManifoldSoA::remove(uint index) {
    freeIndices.insert(index);
    forceFKs.erase(index);
    size--;
}