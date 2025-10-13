#ifndef BODYSOA_H
#define BODYSOA_H

#include "SoA/virtualSoA.h"
#include "util/indexed.h"
#include "util/print.h"

// NOTE we do not need copy or move constructor as we will only have one of these
class BodySoA : public SoA {
private: 
    // xtensors    
    std::vector<Indexed*> bodies;
    std::vector<bool> toDelete;
    std::vector<vec3> pos;
    std::vector<vec3> initial;
    std::vector<vec3> inertial;
    std::vector<vec3> vel;
    std::vector<vec3> prevVel;
    std::vector<vec2> scale;
    std::vector<float> friction;
    std::vector<float> mass;
    std::vector<float> moment;
    std::vector<float> radius;
    std::vector<uint> mesh;
    std::vector<mat2x2> mat;
    std::vector<mat2x2> imat;
    std::vector<mat2x2> rmat;
    std::vector<bool> updated;
    std::vector<ushort> color;
    std::vector<ushort> degree;
    std::vector<ushort> satur;

    // updating forces
    std::vector<uint> oldIndex;
    std::vector<uint> inverseForceMap;

    // solving
    std::vector<vec3> rhs;
    std::vector<mat3x3> lhs;

public:
    BodySoA(uint capacity);
    ~BodySoA() = default;

    void computeTransforms();
    void warmstartBodies(const float dt, const float gravity);
    void updateVelocities(float dt);

    auto& getBodies() { return bodies; }
    auto& getPos() { return pos; }
    auto& getInitial() { return initial; }
    auto& getInertial() { return inertial; }
    auto& getVel() { return vel; }
    auto& getPrevVel() { return prevVel; }
    auto& getScale() { return scale; }
    auto& getFriction() { return friction; }
    auto& getMass() { return mass; }
    auto& getMoment() { return moment; }
    auto& getRadius() { return radius; }
    auto& getMesh() { return mesh; }
    auto& getMat() { return mat; }
    auto& getIMat() { return imat; }
    auto& getRMat() { return rmat; }
    auto& getUpdated() { return updated; }
    auto& getColor() { return color; }
    auto& getDegree() { return degree; }
    auto& getSatur() { return satur; }
    auto& getRHS() { return rhs; }
    auto& getLHS() { return lhs; }
    auto& getInverseForceMap() { return inverseForceMap; }

    void resize(uint newCapacity) override;
    void compact() override;
    uint insert(Indexed* body, vec3 pos, vec3 vel, vec2 scale, float friction, float mass, uint mesh, float radius);
    void remove(uint index);
};

#endif