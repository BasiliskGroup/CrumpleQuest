#ifndef BODYSOA_H
#define BODYSOA_H

#include "SoA/virtualSoA.h"
#include "util/indexed.h"
#include "util/print.h"

// NOTE we do not need copy or move constructor as we will only have one of these
class BodySoA : public SoA {
private: 
    // xtensors    
    xt::xtensor<Indexed*, 1> bodies;
    xt::xtensor<bool, 1> toDelete;

    xt::xtensor<float, 2> pos;
    xt::xtensor<float, 2> initial;
    xt::xtensor<float, 2> inertial;
    xt::xtensor<float, 2> vel;
    xt::xtensor<float, 2> prevVel;
    xt::xtensor<float, 2> scale;

    xt::xtensor<float, 1> friction;
    xt::xtensor<float, 1> mass;
    xt::xtensor<float, 1> moment;
    xt::xtensor<float, 1> radius;

    xt::xtensor<uint, 1> mesh; // vector

    xt::xtensor<mat2x2, 1> mat; // list of 2x2 matrices
    xt::xtensor<mat2x2, 1> imat;

    xt::xtensor<bool, 1> updated;

    xt::xtensor<ushort, 1> color;
    xt::xtensor<ushort, 1> degree;
    xt::xtensor<ushort, 1> satur;

public:
    BodySoA(uint capacity);
    ~BodySoA() = default;

    void computeTransforms();

    xt::xtensor<Indexed*, 1>& getBodies() { return bodies; }
    xt::xtensor<float, 2>& getPos() { return pos; }
    xt::xtensor<float, 2>& getInitial() { return initial; }
    xt::xtensor<float, 2>& getInertial() { return inertial; }
    xt::xtensor<float, 2>& getVel() { return vel; }
    xt::xtensor<float, 2>& getPrevVel() { return prevVel; }
    xt::xtensor<float, 2>& getScale() { return scale; }

    xt::xtensor<float, 1>& getFriction() { return friction; }
    xt::xtensor<float, 1>& getMass() { return mass; }
    xt::xtensor<float, 1>& getMoment() { return moment; }
    xt::xtensor<float, 1>& getRadius() { return radius; }

    xt::xtensor<uint, 1>& getMesh() { return mesh; }

    xt::xtensor<mat2x2, 1>& getMat() { return mat; }
    xt::xtensor<mat2x2, 1>& getIMat() { return imat; }

    xt::xtensor<bool, 1>& getUpdated() { return updated; }

    xt::xtensor<ushort, 1>& getColor() { return color; }
    xt::xtensor<ushort, 1>& getDegree() { return degree; }
    xt::xtensor<ushort, 1>& getSatur() { return satur; }

    void resize(uint newCapacity) override;
    void compact() override;
    uint insert(Indexed* body, vec3 pos, vec3 vel, vec2 scale, float friction, float mass, uint mesh, float radius);
    void remove(uint index);
    void printRigids();
};

#endif