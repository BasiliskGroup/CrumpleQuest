#ifndef SOLVER_H
#define SOLVER_H

#include "util/includes.h"
#include "util/maths.h"

#include "SoA/bodySoA.h"
#include "SoA/forceSoAs.h"
#include "SoA/meshSoA.h"

class Rigid;
class Force;
class Manifold;
class Mesh;

class Solver {
private:
    vec2 gravity;      // Gravity
    int iterations;     // Solver iterations

    float alpha;        // Stabilization parameter
    float beta;         // Penalty ramping parameter
    float gamma;        // Warmstarting decay parameter

    // linked lists heads
    Rigid* bodies;
    Force* forces;

    // SoAs
    ForceSoA* forceSoA;
    BodySoA* bodySoA;
    MeshSoA* meshSoA;

    // broad collision detection
    std::vector<std::pair<uint, uint>> collisionPairs;

    // collision struct for hotloop caching
    struct ColliderRow {
        vec2 pos;
        
        // Exact types from your compiler errors:
        mat2x2 imat;
        vec2* start;
        uint length;
        xt::xview<xt::xtensor<uint, 2>&, uint, xt::xall<size_t>> index;
        
        ColliderRow(float x, float y, mat2x2 imat, vec2* start, uint length, decltype(index) index_view)
            : pos({x, y}), imat(imat), start(start), length(length), index(index_view) {}
    };

    struct CollisionPair {
        uint insertIndex;
        std::vector<vec2> minks;
        vec2 dir;
        uint freeIndex;

        CollisionPair(uint insertIndex, std::vector<vec2> minks, int freeIndex)
            : insertIndex(insertIndex), minks(minks), freeIndex(freeIndex) {}
    };
    
public:
    Solver();
    ~Solver();

    // getters
    Force*& getForces() { return forces; }
    Rigid*& getBodies() { return bodies; }
    ForceSoA* getForceSoA() { return forceSoA; }
    BodySoA*  getBodySoA()  { return bodySoA; }
    MeshSoA*  getMeshSoA()  { return meshSoA; }
    ManifoldSoA* getManifoldSoA() { return forceSoA->getManifoldSoA(); } 

    void step(float dt);
    void draw();

private:
    // getters for accessing rows in a table
    auto& getPos() { return bodySoA->getPos(); }
    auto& getIMat() { return bodySoA->getIMat(); }

    auto& getVerts() { return meshSoA->getVerts(); }
    auto& getStart() { return meshSoA->getStart(); }
    auto& getLength() { return meshSoA->getLength(); }

    uint getStart(uint bodyIndex) { return meshSoA->getStart(bodySoA->getMesh()(bodyIndex)); }
    uint getLength(uint bodyIndex) { return meshSoA->getLength(bodySoA->getMesh()(bodyIndex)); }
    vec2* getStartPtr(uint bodyIndex) { return meshSoA->getStartPtr(bodySoA->getMesh()(bodyIndex)); }

    auto& getIndexA() { return forceSoA->getManifoldSoA()->getIndexA(); }
    auto& getIndexB() { return forceSoA->getManifoldSoA()->getIndexB(); }
    auto& getMinks() { return forceSoA->getManifoldSoA()->getSimplex(); }

    // collision functions
    void sphericalCollision();
    void narrowCollision();
    bool gjk(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void epa();
    void sat();

    // gjk methods helper functions
    void handleSimplex(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void handle0(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void handle1(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void handle2(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void handle3(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void addSupport(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint getFar(vec2* verts, uint length, vec2& dir);

    // sat helper functions
};

#endif