#ifndef FORCE_H
#define FORCE_H

class ForceTable;
class Solver;
class Rigid;

class Force {
private:
    Solver* solver;
    Force* next; // next in solver list
    Force* prev; // prev in solver list
    Force* nextA; // next in body list
    Force* prevA; // prev in body list

    Force* twin; // points to twin force

    Rigid* bodyA;
    Rigid* bodyB;

protected:

    // for table access
    uint index;

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    Force() = default;
    virtual ~Force();

    // getters 
    Solver*& getSolver() { return solver; }
    Force*& getNext()    { return next; }
    Force*& getPrev()    { return prev; }
    Force*& getNextA()   { return nextA; }
    Force*& getPrevA()   { return prevA; }
    Force*& getTwin()    { return twin; }
    Rigid*& getBodyA()   { return bodyA; }
    Rigid*& getBodyB()   { return bodyB; }
    ushort getType();

    void markAsDeleted();
    void disable();

    uint getIndex() { return index; }
    void setIndex(uint index) { this->index = index; }

    // number of jacobian rows (max = 4)
    virtual int rows() const = 0;
    virtual void draw() const {};

    // NOTE initialization and computations will be done in the ForceTable for bulk operations
};

// --------------------------------------------- //
//         TODO add springs and joints           //
// --------------------------------------------- //

#endif