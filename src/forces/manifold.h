#ifndef MANIFOLD_H
#define MANIFOLD_H

#include "forces/force.h"

class Manifold : public Force {
private:

    // tracking the SoA
    uint contactIndex;

public:
    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    ~Manifold();

    int rows() const override { return 4; }
    void draw() const override;
};

#endif