#include "tables/forceRoute.h"
#include "util/print.h"


ManifoldTable::ManifoldTable(ForceTable* forceTable, uint capacity) : forceTable(forceTable) {
    resize(capacity);
}

/**
 * @brief Reserves enough space to the next power of 2 to insert numBodies rows. Assumes that the Table is compact. This function should only be called by the ForceTable
 * 
 * @param numBodies
 * @return the next free index in the Table. 
 */
uint ManifoldTable::reserve(uint numBodies) {
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
void ManifoldTable::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(size, newCapacity,
        toDelete, C0, rA, rB, normal, friction, stick, simplex, forceIndex, tangent, basis, rAW, rBW, cdA, cdB
    );

    // update capacity
    capacity = newCapacity;
}

void ManifoldTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        C0, rA, rB, normal, friction, stick, simplex, forceIndex, tangent, basis, rAW, rBW, cdA, cdB
    );

    size = active;

    // TODO update foreign keys to forceTable
    // reset values for toDelete since they were not mutated in the compact
    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
    }
}

void ManifoldTable::warmstart() {
    // operate only on the first 'size' rows
    for (size_t i = 0; i < size; ++i) {
        tangent[i] = { -normal[i].y, normal[i].x };
        basis[i] = { normal[i], tangent[i] }; // TODO check this constructor
    }
}

void ManifoldTable::remove(uint index) {
    toDelete[index] = true;
}