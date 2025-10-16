#ifndef MANIFOLD_TABLE_H
#define MANIFOLD_TABLE_H

#include "tables/virtualTable.h"

class ForceTable;

// NOTE we do not need copy or move constructor as we will only have one of these
class ManifoldTable : public VirtualTable {
private:
    ForceTable* forceTable;

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
    std::vector<FloatROWS> cdA;
    std::vector<FloatROWS> cdB;

public:
    ManifoldTable(ForceTable* forceTable, uint capacity);
    ~ManifoldTable() = default;

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
    auto& getCdA() { return cdA; }
    auto& getCdB() { return cdB; }
    vec2* getSimplexPtr(uint index) { return simplex[index].data(); }

    uint reserve(uint numBodies);
    void resize(uint new_capacity) override;
    void compact() override;
    void remove(uint index);
};

#endif