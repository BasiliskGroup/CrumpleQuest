#include "solver/physics.h"


Solver::Solver() : forces(nullptr), bodies(nullptr) {
    // set default params
    gravity = { 0, -10 };
    iterations = 10;
    beta = 100000.0f;
    alpha = 0.99f;
    gamma = 0.99f;

    // create SoAs
    forceSoA = new ForceSoA(1024);
    bodySoA = new BodySoA(512);
    meshSoA = new MeshSoA(256, 16);
}

Solver::~Solver() {
    // delete bodies
    while (bodies) {
        delete bodies;
    }

    delete bodySoA;

    // delete forces
    while (forces) {
        delete forces;
    }

    delete forceSoA;
    delete meshSoA;
}

/**
 * @brief Completes a single step for the solver
 * 
 * @param dt time that has passed since last step
 */
void Solver::step(float dt) {
    auto beforeStep = timeNow();
    bodySoA->compact(); // compact bodies to increase cache hits
    printDurationUS(beforeStep, timeNow(), "Body Compact:\t\t");

    auto beforeTransform = timeNow();
    bodySoA->computeTransforms();
    printDurationUS(beforeTransform, timeNow(), "Body Transform:\t\t");

    // NOTE bodies are compact after this point

    auto beforeBroad = timeNow();
    sphericalCollision(); // broad collision TODO replace with BVH
    std::cout << "Broad Pairs:\t\t" << collisionPairs.size() << std::endl;
    printDurationUS(beforeBroad, timeNow(), "Broad Collision:\t");

    auto beforeNarrow = timeNow();
    narrowCollision(); // -> uncompacts manifolds
    printDurationUS(beforeNarrow, timeNow(), "Narrow Collision:\t");

    // warmstart forces -> uncompacts forces
    auto beforeCompact = timeNow();
    forceSoA->compact();
    printDurationUS(beforeCompact, timeNow(), "Force Compact:\t\t");

    auto beforeManifoldWarm = timeNow();
    getManifoldSoA()->warmstart();
    printDurationUS(beforeManifoldWarm, timeNow(), "Manifold Warm:\t\t");

    // NOTE bodies and forces are compact after this point
    print("------------------------------------------");
    printDurationUS(beforeStep, timeNow(), "Total: ");
    print("");
}

// TODO replace this with BVH
void Solver::sphericalCollision() {
    // clear contact pairs from last frame
    collisionPairs.clear();

    uint numBodies = bodySoA->getSize();
    auto pos = bodySoA->getPos();
    auto radii = bodySoA->getRadius();

    float dx;
    float dy;
    float radsum;
    for (Rigid* bodyA = bodies; bodyA != nullptr; bodyA = bodyA->getNext()) {
        for (Rigid* bodyB = bodyA->getNext(); bodyB != nullptr; bodyB = bodyB->getNext()) {
            // ignore collision flag
            if (bodyA->constrainedTo(bodyB)) {
                continue;
            }

            uint i = bodyA->getIndex();
            uint j = bodyB->getIndex();

            dx = pos(i, 0) - pos(j, 0);
            dy = pos(i, 1) - pos(j, 1);
            radsum = radii(i) + radii(j);
            if (radsum * radsum > dx * dx + dy * dy) {
                collisionPairs.emplace_back(i, j);
            }
        }
    }
}

void Solver::narrowCollision() {
    // reserve space to perform all collisions 
    uint forceIndex, manifoldIndex;
    forceSoA->reserveManifolds(collisionPairs.size(), forceIndex, manifoldIndex);

    int count = 0; // TODO debug variable

    for (const auto& pair : collisionPairs) {
        uint rowA = pair.first;
        uint rowB = pair.second;

        // create collider rows for better caching
        ColliderRow a = {
            getPos()(rowA, 0), getPos()(rowA, 1),
            getMat()(rowA), getIMat()(rowA),
            getStartPtr(rowA), getLength(rowA),
            getIndexA().data() + manifoldIndex * getIndexA().shape(1)
        };

        ColliderRow b = {
            getPos()(rowB, 0), getPos()(rowB, 1),
            getMat()(rowB), getIMat()(rowB),
            getStartPtr(rowB), getLength(rowB),
            getIndexB().data() + manifoldIndex * getIndexB().shape(1)
        };

        CollisionPair collisionPair = CollisionPair(forceIndex, manifoldIndex);

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            forceSoA->markForDeletion(forceIndex);
            forceIndex++;
            manifoldIndex++;
            continue;
        }

        // determine collision normal
        count++;
        ushort frontIndex = epa(a, b, collisionPair);
        vec2 normal = collisionPair.polytope[frontIndex].normal;
        getManifoldSoA()->getNormal()(manifoldIndex, 0) = normal.x;
        getManifoldSoA()->getNormal()(manifoldIndex, 1) = normal.y;

        // determine object overlap
        sat(a, b, collisionPair);

        // create manifold force in graph
        forceSoA->getForces()(forceIndex) = new Manifold(this, (Rigid*) bodySoA->getBodies()(rowA), (Rigid*) bodySoA->getBodies()(rowB), forceIndex);

        // increment enumeration
        forceIndex++;
        manifoldIndex++;
    }

    std::cout << "Positive Narrow:\t" << count << std::endl;
}

void Solver::draw() {
    // TODO draw everything
}