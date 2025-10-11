#ifndef MANIFOLDSOA_H
#define MANIFOLDSOA_H

#include "SoA/virtualSoA.h"

class ForceSoA;

// NOTE we do not need copy or move constructor as we will only have one of these
class ManifoldSoA : public SoA {
private:
    ForceSoA* forceSoA;

    // xtensor
    xt::xtensor<bool, 1> toDelete;
    xt::xtensor<float, 3> C0;
    xt::xtensor<float, 3> r;
    xt::xtensor<float, 2> normal;
    xt::xtensor<float, 1> friction;
    xt::xtensor<bool, 1> stick;
    xt::xtensor<vec2, 2> simplex;
    xt::xtensor<uint, 1> forceIndex;

    // arrays for holding extra compute space
    xt::xtensor<float, 2> tangent;
    xt::xtensor<float, 3> basis;
    xt::xtensor<float, 3> rW;

public:
    ManifoldSoA(ForceSoA* forceSoA, uint capacity);
    ~ManifoldSoA() = default;

    void warmstart();

    // getters
    xt::xtensor<float, 2>& getNormal() { return normal; }
    xt::xtensor<float, 3>& getR() { return r; }
    xt::xtensor<uint, 1>& getForceIndex() { return forceIndex; }
    xt::xtensor<vec2, 2>& getSimplex() { return simplex; }

    // setters
    void setRW(const vec2& rW, uint index, ushort subIndex) {
        r(index, subIndex, 0) = rW.x;
        r(index, subIndex, 1) = rW.y;
    }

    uint reserve(uint numBodies);
    void resize(uint new_capacity) override;
    void compact() override;
    void remove(uint index);
};

#endif