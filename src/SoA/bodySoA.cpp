#include "bodySoA.h"

BodySoA::BodySoA(uint capacity) {
    this->capacity = capacity;

    // fill freeIndices
    for (uint i = 0; i < capacity; ++i) {
        freeIndices.insert(i);
    }

    // create all xtensors
    pos      = xt::xtensor<float, 2>::from_shape({capacity, 3});
    initial  = xt::xtensor<float, 2>::from_shape({capacity, 3});
    inertial = xt::xtensor<float, 2>::from_shape({capacity, 3});
    vel      = xt::xtensor<float, 2>::from_shape({capacity, 3});
    prevVel  = xt::xtensor<float, 2>::from_shape({capacity, 3});

    scale    = xt::xtensor<float, 2>::from_shape({capacity, 2});

    friction = xt::xtensor<float, 1>::from_shape({capacity});
    radius   = xt::xtensor<float, 1>::from_shape({capacity});
    mass     = xt::xtensor<float, 1>::from_shape({capacity});
    moment   = xt::xtensor<float, 1>::from_shape({capacity});

    mesh = xt::xtensor<unsigned int, 1>::from_shape({capacity});

    mat  = xt::xtensor<mat2x2, 1>::from_shape({capacity});
    imat = xt::xtensor<mat2x2, 1>::from_shape({capacity});

    updated = xt::xtensor<bool, 1>::from_shape({capacity});

    color  = xt::xtensor<unsigned short, 1>::from_shape({capacity});
    degree = xt::xtensor<unsigned short, 1>::from_shape({capacity});
    satur  = xt::xtensor<unsigned short, 1>::from_shape({capacity});
}

/**
 * @brief Computes the scale and rotation matrix and its inverse. Assumes the tensor is compact.
 * 
 */
void BodySoA::computeTransforms() {
    for (uint i = 0; i < size; i++) {
        if (!updated(i)) continue;

        float angle = pos(i, 2);
        float sx = scale(i, 0), sy = scale(i, 1);
        float isx = 1 / sx, isy = 1 / sy;

        float c = cos(angle);
        float s = sin(angle);

        mat(i) = { c * sx, -s * sy, s * sx, c * sy };
        imat(i) = { c * isx, s * isy, -s * isx, c * isy };

        updated(i) = false;
    }
}

/**
 * @brief Resizes each tensor in the system up to the specified size. 
 * 
 * @param newCapacity new capacity of the tensor. If this is below the current size, the function is ignored. 
 */
void BodySoA::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensor(pos,      size, newCapacity);
    expandTensor(initial,  size, newCapacity);
    expandTensor(inertial, size, newCapacity);
    expandTensor(vel,      size, newCapacity);
    expandTensor(prevVel,  size, newCapacity);
    expandTensor(scale,    size, newCapacity);
    expandTensor(friction, size, newCapacity);
    expandTensor(radius,   size, newCapacity);
    expandTensor(mass,     size, newCapacity);
    expandTensor(moment,   size, newCapacity);
    expandTensor(mesh,     size, newCapacity);
    expandTensor(mat,      size, newCapacity);
    expandTensor(imat,     size, newCapacity);
    expandTensor(updated,  size, newCapacity);
    expandTensor(color,    size, newCapacity);
    expandTensor(degree,   size, newCapacity);
    expandTensor(satur,    size, newCapacity);

    // add all new indices to freeIndices
    for (uint i = size; i < newCapacity; ++i) {
        freeIndices.insert(i);
    }

    // update capacity
    capacity = newCapacity;
}

// NOTE this function is very expensive but should only be called once per frame
// if needed, find a cheaper solution
void BodySoA::compact() {
    if (freeIndices.empty()) return; // nothing to remove

    compactTensors(freeIndices, capacity, bodies,
        pos, initial, inertial, vel, prevVel,
        scale, friction, radius, mass, moment,
        mesh, mat, imat, updated, color, degree, satur
    );

    // update maps
    for (const auto& pair : bodies) {
        pair.second->setIndex(pair.first);
    }

    // update freeIndices
    freeIndices.clear();
    for (uint i = size; i < capacity; i++) {
        freeIndices.insert(i);
    }
}

int BodySoA::insert(vec3 pos, vec3 vel, vec2 scale, float friction, float mass, uint mesh, float radius) {
    if (freeIndices.empty()) {
        resize(capacity * 2);
    }

    // pop a random element from the set
    auto it = freeIndices.begin();
    uint index = *it;
    freeIndices.erase(it);

    // insert into row
    this->pos(index, 0) = pos.x;
    this->pos(index, 1) = pos.y;
    this->pos(index, 2) = pos.z;

    this->vel(index, 0) = vel.x;
    this->vel(index, 1) = vel.y;
    this->vel(index, 2) = vel.z;

    this->scale(index, 0) = scale.x;
    this->scale(index, 1) = scale.y;

    this->friction(index) = friction;
    this->mass(index) = mass;
    this->mesh(index) = mesh;
    this->radius(index) = radius;

    this->updated(index) = false;

    // increment size
    size += 1;
    return index;
}

void BodySoA::remove(uint index) {
    freeIndices.insert(index);
    bodies.erase(index);
    size--;
}

void BodySoA::printRigids() {
    for(uint i = 0; i < size; i++) {
        std::cout << "(" << pos(i, 0) << ", " << pos(i, 1) << ", " << pos(i, 2) << ")" << " <" << vel(i, 0) << ", " << vel(i, 1) << ", " << vel(i, 2) << ">" << std::endl;
    }
}