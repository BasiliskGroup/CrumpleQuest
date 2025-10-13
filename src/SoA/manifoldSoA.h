#ifndef MANIFOLDSOA_H
#define MANIFOLDSOA_H

#include "SoA/virtualSoA.h"

class ForceSoA;

// NOTE we do not need copy or move constructor as we will only have one of these
class ManifoldSoA : public SoA {
private:
    ForceSoA* forceSoA;

    // xtensor
    std::vector<bool> toDelete;
    std::vector<Vec2Pair> C0;
    std::vector<Vec2Pair> rA;
    std::vector<Vec2Pair> rB;
    std::vector<vec2> normal;
    std::vector<float> friction;
    std::vector<bool> stick;
    std::vector<Vec2Triplet> simplex;
    std::vector<uint> forceIndex;

    // arrays for holding extra compute space
    std::vector<vec2> tangent;
    std::vector<mat2x2> basis;
    std::vector<Vec2Pair> rAW;
    std::vector<Vec2Pair> rBW;
    std::vector<vec3> dpA;
    std::vector<vec3> dpB;

public:
    ManifoldSoA(ForceSoA* forceSoA, uint capacity);
    ~ManifoldSoA() = default;

    void warmstart();

    // getters
    auto& getBasis() { return basis; }
    auto& getTangent() { return tangent; }
    auto& getNormal() { return normal; }
    auto& getRA() { return rA; }
    auto& getRB() { return rB; } 
    auto& getRAW() { return rAW; } 
    auto& getRBW() { return rBW; }
    auto& getC0() { return C0; }
    auto& getForceIndex() { return forceIndex; }
    auto& getSimplex() { return simplex; }
    auto& getFriction() { return friction; }
    auto& getDpA() { return dpA; }
    auto& getDpB() { return dpB; }
    vec2* getSimplexPtr(uint index) { return simplex[index].data(); }

    uint reserve(uint numBodies);
    void resize(uint new_capacity) override;
    void compact() override;
    void remove(uint index);
};

#endif