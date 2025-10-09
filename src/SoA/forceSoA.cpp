#include "SoA/forceSoAs.h"
#include "util/constants.h"
#include "util/print.h"


ForceSoA::ForceSoA(uint capacity) {
    this->capacity = capacity;

    // create all xtensors
    forces = xt::xtensor<Indexed*, 1>::from_shape({capacity});
    toDelete = xt::xtensor<bool, 1>::from_shape({capacity});
    JA = xt::xtensor<float, 3>::from_shape({capacity, ROWS, 3});
    JB = xt::xtensor<float, 3>::from_shape({capacity, ROWS, 3});
    HA = xt::xtensor<float, 4>::from_shape({capacity, ROWS, 3, 3});
    HB = xt::xtensor<float, 4>::from_shape({capacity, ROWS, 3, 3});
    C         = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    motor     = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    stiffness = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    fracture  = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    fmax      = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    fmin      = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    penalty   = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    lambda    = xt::xtensor<float, 2>::from_shape({capacity, ROWS});
    type = xt::xtensor<ushort, 1>::from_shape({capacity});
    specialIndex = xt::xtensor<uint, 1>::from_shape({capacity});
    bodyIndex    = xt::xtensor<uint, 1>::from_shape({capacity});

    // create SoAs
    manifoldSoA = new ManifoldSoA(this, capacity);
}

ForceSoA::~ForceSoA() {
    delete manifoldSoA;
}

void ForceSoA::reserveManifolds(uint numPairs, uint& forceIndex, uint& manifoldIndex) {
    manifoldIndex = manifoldSoA->reserve(numPairs);
    uint neededSpace = pow(2, ceil(log2(size + numPairs)));

    if (neededSpace >= capacity) {
        resize(neededSpace);
    }

    // ensure reserved slots arent deleted
    for (uint i = size; i < size + numPairs; i++) {
        toDelete(i) = false;
    }

    forceIndex = size;
    size += numPairs;
}

void ForceSoA::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(size, newCapacity, 
        forces, toDelete, JA, JB, C, motor, stiffness, fracture, fmax, fmin, penalty, lambda, HA, HB, type, specialIndex, bodyIndex
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

    // todo write new compact function
    compactTensors(toDelete, size, 
        forces, JA, JB, C, motor, stiffness, 
        fracture, fmax,fmin, penalty, lambda, 
        HA, HB, type, specialIndex, bodyIndex
    );

    size = active;

    // update maps
    for (uint i = 0; i < size; i++) {
        // update force object
        // TODO add in force handling for graphs
        // if (forces(i) != nullptr) forces(i)->setIndex(i);

        // reset values for toDelete since they were not mutated in the compact
        toDelete(i) = false;

        // TODO determine if we need back references from foreign tables
        // switch (type(i)) {
        //     case 0: // null
        //         break; 
        //     case 1: // manifold
        //         manifoldSoA->getForceIndex()(specialIndex(i)) = i;
        //         break;
        //     case 2: // joint
        //         break; 
        //     case 3: // spring
        //         break; 
        //     case 4: // ignoreCollision
        //         break; 
        //     default:
        //         throw std::runtime_error("Force not recognized.");
        // }
    }

    // compact special tables
    manifoldSoA->compact();
}

int ForceSoA::insert() {
    // Skipped as requested - you'll handle the special cases
    return 0;
}

void ForceSoA::remove(uint index) {
    toDelete(index) = true;
}