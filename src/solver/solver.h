#ifndef SOLVER_H
#define SOLVER_H

#include "util/includes.h"
#include "util/maths.h"
#include "util/time.h"
#include "util/print.h"

#include "SoA/bodySoA.h"
#include "SoA/forceSoAs.h"
#include "SoA/meshSoA.h"


class Rigid;
class Force;
class Manifold;
class Mesh;

class Solver {
private:
    float gravity;      // Gravity
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
        vec2 scale;
        mat2x2 mat;
        mat2x2 imat;
        vec2* start;
        uint length;
        vec2* simplex;
        std::array<float, 4> dots; // TODO this needs to be at least the length of the mesh

        ColliderRow() = default;
    };

    struct PolytopeFace {
        vec2 normal;
        float distance;

        // face vertices
        ushort va;
        ushort vb;

        PolytopeFace() = default;
        PolytopeFace(ushort va, ushort vb, vec2 normal, float distance)
            : normal(normal), distance(distance), va(va), vb(vb) {}
    };

    using Simplex = std::array<vec2, 3>;
    using SpSet = std::array<ushort, EPA_ITERATIONS + 3>;
    using SpArray = std::array<vec2, EPA_ITERATIONS + 3>;
    using Polytope = std::array<PolytopeFace, EPA_ITERATIONS + 3>;

    struct CollisionPair {
        uint forceIndex;
        uint manifoldIndex;

        // gjk
        Simplex minks;
        vec2 dir;

        // epa
        // we add 3 since we start with 3 faces
        SpArray sps;
        SpSet spSet;
        Polytope polytope;

        CollisionPair() = default;
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
    // manage storage functions
    void compactBodies();
    void compactForces();
    void reserveForcesForCollision(uint& forceIndex, uint& manifoldIndex);

    // compute stages
    void warmstartManifolds();
    void warmstartForces();
    void warmstartBodies(float dt);
    void updateVelocities(float dt);
    void mainloopPreload();
    void primalUpdate(float dt);
    void dualUpdate(float dt);

    void computeConstraints(uint start, uint end, ushort type);
    void computeDerivatives(uint start, uint end, ushort type);
    void loadCdX(uint start, uint end);

    // collision functions
    void sphericalCollision();
    void narrowCollision();
    
    bool gjk(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex);
    ushort epa(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void sat(ColliderRow& a, ColliderRow& b, CollisionPair& pair);

    void initColliderRow(uint row, uint manifoldIndex, ColliderRow& colliderRow);

    // gjk methods helper functions
    uint handleSimplex(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex);
    uint handle0(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint handle1(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint handle2(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint handle3(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void addSupport(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex);
    void getFar(const ColliderRow& row, const vec2& dir, vec2& simplexLocal);

    // epa helper methods
    ushort insertHorizon(SpSet& spSet, ushort spIndex, ushort setSize);
    bool discardHorizon(SpSet& spSet, ushort spIndex, ushort setSize);
    ushort polytopeFront(const Polytope& polytope, ushort numFaces);
    void removeFace(Polytope& polytope, ushort index, ushort numFaces);
    void supportSpOnly(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex);
    void buildFace(CollisionPair& pair, ushort indexA, ushort indexB, ushort indexL);

    using Dots = std::array<float, 4>;

    // sat helper functions
    void intersect(ColliderRow& a, ColliderRow& b, CollisionPair& pair, const vec2& mtv);
    void clampToEdge(const vec2& edge, vec2& toClamp);
    void dotEdgeIntersect(const vec2* verts, uint start, Dots dots, float thresh);

    template <typename Compare> // std::greater_equal<int>()) // std::less_equal<int>()) // if (cmp(a, b))
    void findBounds(Dots dots, const float thresh, uint& begin, uint& end, Compare cmp);

    template <typename Compare>
    void findExtremes(Dots dots, uint& begin, uint& end, Compare cmp);
};

#endif