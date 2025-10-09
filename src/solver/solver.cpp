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
    // delete linked lists
    while (forces) {
        delete forces;
    }

    while (bodies) {
        delete bodies;
    }

    // delete SoAs
    delete forceSoA;
    delete bodySoA;
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
    printDurationUS(beforeStep, timeNow(), "Body Compact: ");

    auto beforeTransform = timeNow();
    bodySoA->computeTransforms();
    printDurationUS(beforeTransform, timeNow(), "Body Transform: ");

    // NOTE bodies are compact after this point

    auto beforeBroad = timeNow();
    sphericalCollision(); // broad collision TODO replace with BVH
    std::cout << "Broad Pairs: " << collisionPairs.size() << std::endl;
    printDurationUS(beforeBroad, timeNow(), "Broad Collision: ");

    auto beforeNarrow = timeNow();
    narrowCollision(); // -> uncompacts manifolds
    printDurationUS(beforeNarrow, timeNow(), "Narrow Collision: ");

    // warmstart forces -> uncompacts forces
    auto beforeCompact = timeNow();
    forceSoA->compact();
    printDurationUS(beforeCompact, timeNow(), "Force Compact: ");

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
    for (uint i = 0; i < numBodies; i++) {
        for (uint j = i + 1; j < numBodies; j++) {
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
    uint insertIndex = 0;
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
            xt::view(getIndexA(), manifoldIndex + insertIndex, xt::all())
        };

        ColliderRow b = {
            getPos()(rowB, 0), getPos()(rowB, 1),
            getMat()(rowB), getIMat()(rowB),
            getStartPtr(rowB), getLength(rowB),
            xt::view(getIndexB(), manifoldIndex + insertIndex, xt::all())
        };

        CollisionPair collisionPair = CollisionPair();

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            forceSoA->markForDeletion(insertIndex + forceIndex);
            insertIndex++;
            continue;
        }

        // determine collision normal
        count++;
        ushort frontIndex = epa(a, b, collisionPair);
        // print(collisionPair.polytope[frontIndex].normal);

        // determine object overlap

        // create manifold force in graph
        new Manifold(this, (Rigid*) bodySoA->getBodies()(rowA), (Rigid*) bodySoA->getBodies()(rowB), insertIndex + forceIndex);

        // increment enumeration
        insertIndex++;
    }

    std::cout << "Narrow: " << count << std::endl;
}

void Solver::draw() {
    // TODO draw everything
}