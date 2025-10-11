#ifndef FORCESOA_H
#define FORCESOA_H

#include "SoA/virtualSoA.h"
#include "util/indexed.h"

class ManifoldSoA;

// NOTE we do not need copy or move constructor as we will only have one of these
class ForceSoA : public SoA {
private:
    ManifoldSoA* manifoldSoA;

    // xtensor TODO fix dimensions
    xt::xtensor<Indexed*, 1> forces;
    xt::xtensor<bool, 1> toDelete;
    xt::xtensor<float, 3> J;
    xt::xtensor<float, 4> H;
    xt::xtensor<float, 2> C;
    xt::xtensor<float, 2> motor;
    xt::xtensor<float, 2> stiffness;
    xt::xtensor<float, 2> fracture;
    xt::xtensor<float, 2> fmax;
    xt::xtensor<float, 2> fmin;
    xt::xtensor<float, 2> penalty;
    xt::xtensor<float, 2> lambda;
    xt::xtensor<ushort, 1> type;
    xt::xtensor<uint, 1> specialIndex;
    xt::xtensor<uint, 1> bodyIndex;

public:
    ForceSoA(uint capacity);
    ~ForceSoA();

    void markForDeletion(uint index);
    
    xt::xtensor<bool, 1>& getToDelete() { return toDelete; }
    xt::xtensor<uint, 1>& getBodyIndex() { return bodyIndex; }
    xt::xtensor<uint, 1>& getSpecial() { return specialIndex; }
    xt::xtensor<Indexed*, 1>& getForces() { return forces; }
    ManifoldSoA* getManifoldSoA() { return manifoldSoA; }

    void reserveManifolds(uint numBodies, uint& forceIndex, uint& manifoldIndex);
    void resize(uint newCapacity) override;
    void compact() override;
    int insert();
    void remove(uint index);
};

#endif