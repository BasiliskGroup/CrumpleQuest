#ifndef FORCE_H
#define FORCE_H

#include "util/indexed.h"

class ForceSoA;
class Solver;
class Rigid;

class Force : public Indexed {
private:
    Solver* solver;
    Force* next;
    Force* nextA;
    Force* nextB;
    Rigid* bodyA;
    Rigid* bodyB;

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    Force() = default;
    virtual ~Force();

    // getters 
    Force*& getNext()  { return next; }
    Force*& getNextA() { return nextA; }
    Force*& getNextB() { return nextB; }
    Rigid*& getBodyA() { return bodyA; }
    Rigid*& getBodyB() { return bodyB; }
    ForceSoA* getForceSoA();

    void disable();

    // number of jacobian rows (max = 4)
    virtual int rows() const = 0;
    virtual void draw() const {};

    // NOTE initialization and computations will be done in the ForceSoA for bulk operations
};

// --------------------------------------------- //
//         TODO add springs and joints           //
// --------------------------------------------- //

#endif