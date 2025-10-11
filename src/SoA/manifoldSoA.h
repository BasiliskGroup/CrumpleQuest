#ifndef MANIFOLDSOA_H
#define MANIFOLDSOA_H

#include "SoA/virtualSoA.h"

class ForceSoA;

enum JType {
    JN1,
    JT1,
    JN2,
    JT2
};

// NOTE we do not need copy or move constructor as we will only have one of these
class ManifoldSoA : public SoA {
private:
    ForceSoA* forceSoA;

    // xtensor
    std::vector<bool> toDelete;
    std::vector<Vec2Pair> C0;
    std::vector<Vec2Pair> r;
    std::vector<vec2> normal;
    std::vector<float> friction;
    std::vector<bool> stick;
    std::vector<Vec2Triplet> simplex;
    std::vector<uint> forceIndex;

    // arrays for holding extra compute space
    std::vector<vec2> tangent;
    std::vector<mat2x2> basis;
    std::vector<Vec2Pair> rW;

public:
    ManifoldSoA(ForceSoA* forceSoA, uint capacity);
    ~ManifoldSoA() = default;

    void warmstart();

    // getters
    auto& getNormal() { return normal; }
    auto& getR() { return r; }
    auto& getForceIndex() { return forceIndex; }
    auto& getSimplex() { return simplex; }
    vec2* getSimplexPtr(uint index) { return simplex[index].data(); }

    // setters
    void setRW(const vec2& rW, uint index, ushort subIndex) { r[index][subIndex] = rW; }

    uint reserve(uint numBodies);
    void resize(uint new_capacity) override;
    void compact() override;
    void remove(uint index);
};

#endif