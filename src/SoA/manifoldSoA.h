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
    xt::xtensor<float, 3> rA;
    xt::xtensor<float, 3> rB;
    xt::xtensor<float, 2> normal;
    xt::xtensor<float, 1> friction;
    xt::xtensor<bool, 1> stick;
    xt::xtensor<uint, 2> indexA;
    xt::xtensor<uint, 2> indexB;
    xt::xtensor<float, 3> simplex;
    xt::xtensor<uint, 1> forceIndex;

public:
    ManifoldSoA(ForceSoA* forceSoA, uint capacity);
    ~ManifoldSoA() = default;

    xt::xtensor<uint, 1>& getForceIndex() { return forceIndex; }
    xt::xtensor<uint, 2>& getIndexA() { return indexA; }
    xt::xtensor<uint, 2>& getIndexB() { return indexB; }
    xt::xtensor<float, 3>& getSimplex() { return simplex; }

    uint reserve(uint numPairs);
    void resize(uint new_capacity) override;
    void compact() override;
    void insert();
    void remove(uint index);
};

#endif