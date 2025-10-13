#ifndef FORCESOA_H
#define FORCESOA_H

#include "SoA/virtualSoA.h"
#include "util/indexed.h"

class ManifoldSoA;

// NOTE we do not need copy or move constructor as we will only have one of these
class ForceSoA : public SoA {
private:
    ManifoldSoA* manifoldSoA;

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
    ForceSoA(uint capacity);
    ~ForceSoA();

    void markForDeletion(uint index);
    
    auto& getIsA() { return isA; }
    auto& getJ() { return J; }
    auto& getH() { return H; }
    auto& getC() { return C; }
    auto& getToDelete() { return toDelete; }
    auto& getBodyIndex() { return bodyIndex; }
    auto& getSpecial() { return specialIndex; }
    auto& getForces() { return forces; }
    ManifoldSoA* getManifoldSoA() { return manifoldSoA; }

    void reserveManifolds(uint numPairs, uint& forceIndex, uint& manifoldIndex);
    void resize(uint newCapacity) override;
    void compact() override;
    int insert();
    void remove(uint index);
};

#endif