#ifndef FORCE_H
#define FORCE_H

#include "util/indexed.h"

class ForceTable;
class Solver;
class Rigid;

class Force : public Indexed {
private:
    Solver* solver;
    Force* next;
    Force* nextA;
    Force* twin;
    Rigid* bodyA;
    Rigid* bodyB;

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    Force() = default;
    virtual ~Force();

    // getters 
    Force*& getNext()  { return next; }
    Force*& getNextA() { return nextA; }
    Rigid*& getBodyA() { return bodyA; }
    Rigid*& getBodyB() { return bodyB; }
    ushort getType();
    ForceTable* getForceTable();

    // setters
    void setTwin(Force* twin) { this->twin = twin; }

    void markForDeletion();
    void unlink();
    void disable();

    // number of jacobian rows (max = 4)
    virtual int rows() const = 0;
    virtual void draw() const {};

    // NOTE initialization and computations will be done in the ForceTable for bulk operations
};

// --------------------------------------------- //
//         TODO add springs and joints           //
// --------------------------------------------- //

#endif