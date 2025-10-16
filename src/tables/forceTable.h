#ifndef FORCE_TABLE_H
#define FORCE_TABLE_H

#include "tables/virtualTable.h"
#include "util/indexed.h"

class ManifoldTable;

// NOTE we do not need copy or move constructor as we will only have one of these
class ForceTable : public VirtualTable {
private:
    ManifoldTable* manifoldTable;

    std::vector<Vec3ROWS> J;
    std::vector<Mat3x3ROWS> H;
    std::vector<FloatROWS> C;
    std::vector<FloatROWS> motor;
    std::vector<FloatROWS> stiffness;
    std::vector<FloatROWS> fracture;
    std::vector<FloatROWS> fmax;
    std::vector<FloatROWS> fmin;
    std::vector<FloatROWS> penalty;
    std::vector<FloatROWS> lambda;

    std::vector<Indexed*> forces;
    std::vector<bool> toDelete;
    std::vector<ushort> type;

    std::vector<uint> specialIndex;
    std::vector<uint> bodyIndex;
    std::vector<bool> isA;

public:
    ForceTable(uint capacity);
    ~ForceTable();

    void markAsDeleted(uint index);
    void warmstart(float alpha, float gamma);
    
    auto& getIsA() { return isA; }
    auto& getJ() { return J; }
    auto& getH() { return H; }
    auto& getC() { return C; }
    auto& getFmax() { return fmax; }
    auto& getFmin() { return fmin; }
    auto& getLambda() { return lambda; }
    auto& getStiffness() { return stiffness; }
    auto& getPenalty() { return penalty; }
    auto& getMotor() { return motor; }
    auto& getToDelete() { return toDelete; }
    auto& getBodyIndex() { return bodyIndex; }
    auto& getSpecial() { return specialIndex; }
    auto& getForces() { return forces; }
    auto& getType() { return type; } 
    ManifoldTable* getManifoldTable() { return manifoldTable; }

    void reserveManifolds(uint numPairs, uint& forceIndex, uint& manifoldIndex);
    void resize(uint newCapacity) override;
    void compact() override;
    int insert();
};

#endif