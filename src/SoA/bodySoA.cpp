#include "bodySoA.h"

BodySoA::BodySoA(uint capacity) {
    this->capacity = capacity;

    // create all xtensors
    bodies = xt::xtensor<Indexed*, 1>::from_shape({capacity});
    toDelete = xt::xtensor<bool, 1>::from_shape({capacity});

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
        if (updated(i)) continue;

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

    expandTensors(size, newCapacity,
        bodies, toDelete, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, mesh, mat, imat, updated, color, degree, satur
    );

    // update capacity
    capacity = newCapacity;
}

// NOTE this function is very expensive but should only be called once per frame
// if needed, find a cheaper solution
void BodySoA::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        bodies, pos, initial, inertial, vel, prevVel,
        scale, friction, radius, mass, moment,
        mesh, mat, imat, updated, color, degree, satur
    );

    size = active;

    for (uint i = 0; i < size; i++) {
        toDelete(i) = false;
    }
}

uint BodySoA::insert(Indexed* body, vec3 pos, vec3 vel, vec2 scale, float friction, float mass, uint mesh, float radius) {
    if (size == capacity) {
        resize(capacity * 2);
    }

    // insert into row
    this->bodies(size) = body;
    this->toDelete(size) = false;

    this->pos(size, 0) = pos.x;
    this->pos(size, 1) = pos.y;
    this->pos(size, 2) = pos.z;

    this->vel(size, 0) = vel.x;
    this->vel(size, 1) = vel.y;
    this->vel(size, 2) = vel.z;

    this->scale(size, 0) = scale.x;
    this->scale(size, 1) = scale.y;

    this->friction(size) = friction;
    this->mass(size) = mass;
    this->mesh(size) = mesh;
    this->radius(size) = radius;

    this->updated(size) = false;

    // increment size
    return size++;
}

void BodySoA::remove(uint index) {
    toDelete(index) = true;
}

void BodySoA::printRigids() {
    for(uint i = 0; i < size; i++) {
        std::cout << "(" << pos(i, 0) << ", " << pos(i, 1) << ", " << pos(i, 2) << ")" << " <" << vel(i, 0) << ", " << vel(i, 1) << ", " << vel(i, 2) << ">" << std::endl;
    }
}