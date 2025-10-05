#include "SoA/forceSoAs.h"
#include "util/constants.h"

ForceSoA::ForceSoA(uint capacity) {
    this->capacity = capacity;

    // fill freeIndices
    for (uint i = 0; i < capacity; ++i) {
        freeIndices.insert(i);
    }

    // create all xtensors
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

void ForceSoA::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensor(JA,           size, newCapacity);
    expandTensor(JB,           size, newCapacity);
    expandTensor(C,            size, newCapacity);
    expandTensor(motor,        size, newCapacity);
    expandTensor(stiffness,    size, newCapacity);
    expandTensor(fracture,     size, newCapacity);
    expandTensor(fmax,         size, newCapacity);
    expandTensor(fmin,         size, newCapacity);
    expandTensor(penalty,      size, newCapacity);
    expandTensor(lambda,       size, newCapacity);
    expandTensor(HA,           size, newCapacity);
    expandTensor(HB,           size, newCapacity);
    expandTensor(type,         size, newCapacity);
    expandTensor(specialIndex, size, newCapacity);
    expandTensor(bodyIndex,    size, newCapacity);

    // add all new indices to freeIndices
    for (uint i = capacity; i < newCapacity; ++i) {
        freeIndices.insert(i);
    }

    // update capacity
    capacity = newCapacity;
}

void ForceSoA::compact() {
    if (freeIndices.empty()) return; // nothing to remove

    compactTensors(freeIndices, capacity, forces,
        JA, JB, C, motor, stiffness, fracture,
        fmax,fmin, penalty, lambda, HA, HB,
        type, specialIndex, bodyIndex
    );

    // update maps
    for (const auto& pair : forces) {
        pair.second->setIndex(pair.first);
    }

    auto manifoldFKs = manifoldSoA->getForceIndex();
    for (const auto& pair : specialFKs) {
        switch (type(pair.first)) {
            case 0: // null
                break; 
            case 1: // manifold
                manifoldFKs(pair.second) = pair.first;
                break;
            case 2: // joint
                break; 
            case 3: // spring
                break; 
            case 4: // ignoreCollision
                break; 
            default:
                throw std::runtime_error("Force not recognized.");
        }
    }

    // update freeIndices
    freeIndices.clear();
    for (uint i = size; i < capacity; i++) {
        freeIndices.insert(i);
    }

    // compact special tables
    manifoldSoA->compact();
}

int ForceSoA::insert() {
    // Skipped as requested - you'll handle the special cases
    return 0;
}

void ForceSoA::remove(uint index) {
    freeIndices.insert(index);
    forces.erase(index);
    size--;
}