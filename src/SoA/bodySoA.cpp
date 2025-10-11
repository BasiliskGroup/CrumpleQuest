#include "bodySoA.h"

BodySoA::BodySoA(uint capacity) {
    this->capacity = capacity;

    // create all xtensors
    bodies.resize(capacity);
    toDelete.resize(capacity); 
    pos.resize(capacity); 
    initial.resize(capacity);
    inertial.resize(capacity); 
    vel.resize(capacity);  
    prevVel.resize(capacity);
    scale.resize(capacity);   
    friction.resize(capacity); 
    radius.resize(capacity); 
    mass.resize(capacity);  
    moment.resize(capacity);  
    mesh.resize(capacity); 
    mat.resize(capacity);
    imat.resize(capacity);
    rmat.resize(capacity);

    updated.resize(capacity);
    color.resize(capacity);
    degree.resize(capacity);
    satur.resize(capacity);

    oldIndex.resize(capacity);
    inverseForceMap.resize(capacity);
}

/**
 * @brief Computes the scale and rotation matrix and its inverse. Assumes the tensor is compact.
 * 
 */
void BodySoA::computeTransforms() {
    for (uint i = 0; i < size; i++) {
        if (updated[i]) continue;

        float angle = pos[i].z;
        float sx = scale[i].x, sy = scale[i].y;
        float isx = 1 / sx, isy = 1 / sy;

        float c = cos(angle);
        float s = sin(angle);

        rmat[i] = { c, -s, s, c };
        mat[i] = { c * sx, -s * sy, s * sx, c * sy };
        imat[i] = { c * isx, s * isy, -s * isx, c * isy };

        updated[i] = false;
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
        bodies, toDelete, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, mesh, mat, imat, rmat, updated, color, degree, satur, oldIndex, inverseForceMap
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

    // reset old indices
    for (uint i = 0; i < size; i++) {
        oldIndex[i] = i;
    }

    // TODO check to see who needs to be compacted and who will just get cleared anyway
    compactTensors(toDelete, size,
        bodies, pos, initial, inertial, vel, prevVel,
        scale, friction, radius, mass, moment,
        mesh, mat, imat, rmat, updated, color, degree, satur, oldIndex
    );

    // invert old indices so that forces can find their new indices
    for (uint i = 0; i < size; i++) {
        inverseForceMap[oldIndex[i]] = i;
    }

    size = active;

    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
        bodies[i]->setIndex(i);
    }
}

uint BodySoA::insert(Indexed* body, vec3 pos, vec3 vel, vec2 scale, float friction, float mass, uint mesh, float radius) {
    if (size == capacity) {
        resize(capacity * 2);
    }

    // insert into row
    this->bodies[size] = body;
    this->toDelete[size] = false;
    this->pos[size] = pos;
    this->vel[size] = vel;
    this->scale[size] = scale;
    this->friction[size] = friction;
    this->mass[size] = mass;
    this->mesh[size] = mesh;
    this->radius[size] = radius;
    this->updated[size] = false;

    // increment size
    return size++;
}

void BodySoA::remove(uint index) {
    toDelete[index] = true;
}