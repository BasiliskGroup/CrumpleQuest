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
    compactBodies();
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
    compactForces();
    printDurationUS(beforeCompact, timeNow(), "Force Compact:\t\t");

    auto beforeManifoldWarm = timeNow();
    getManifoldSoA()->warmstart();
    printDurationUS(beforeManifoldWarm, timeNow(), "Manifold Warm:\t\t");

    // NOTE bodies and forces are compact after this point
    print("------------------------------------------");
    printDurationUS(beforeStep, timeNow(), "Total: ");
    print("");
}

void Solver::compactBodies() {
    bodySoA->compact();

    // route forces back to their correct bodies after standard compact
    auto bodyIndices = forceSoA->getBodyIndex();
    auto toDelete = forceSoA->getToDelete();
    auto inverseForceMap = bodySoA->getInverseForceMap();

    for (uint i = 0; i < forceSoA->getSize(); i++) {
        // will index out of bounds
        if (toDelete(i) == true) {
            continue;
        }
        bodyIndices(i) = inverseForceMap(bodyIndices(i));
    }
}

void Solver::compactForces() {
    forceSoA->compact();
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
    reserveForcesForCollision(forceIndex, manifoldIndex);

    int count = 0; // TODO debug variable

    for (const auto& pair : collisionPairs) {
        uint rowA = pair.first;
        uint rowB = pair.second;

        ColliderRow a, b;
        CollisionPair collisionPair;
        initColliderRow(rowA, manifoldIndex, a);
        initColliderRow(rowB, manifoldIndex, b);
        collisionPair.forceIndex = forceIndex;
        collisionPair.manifoldIndex = manifoldIndex;

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            forceSoA->markForDeletion(forceIndex + 0);
            forceSoA->markForDeletion(forceIndex + 1);
            forceIndex += 2;
            manifoldIndex += 2;
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
        // TODO delay creation of these forces until after multithreading, these will insert into linked lists creating race conditions
        forceSoA->getForces()(forceIndex + 0) = new Manifold(this, (Rigid*) bodySoA->getBodies()(rowA), (Rigid*) bodySoA->getBodies()(rowB), forceIndex + 0); // A -> B
        forceSoA->getForces()(forceIndex + 1) = new Manifold(this, (Rigid*) bodySoA->getBodies()(rowB), (Rigid*) bodySoA->getBodies()(rowA), forceIndex + 1); // B -> A

        // increment enumeration
        forceIndex += 2;
        manifoldIndex += 2;
    }

    std::cout << "Positive Narrow:\t" << count << std::endl;
}

void Solver::reserveForcesForCollision(uint& forceIndex, uint& manifoldIndex) {
    forceSoA->reserveManifolds(collisionPairs.size() * 2, forceIndex, manifoldIndex);

    // assign forces their bodies
    auto& bodyIndices = forceSoA->getBodyIndex();
    uint i = 0;
    for (const auto& pair : collisionPairs) {
        bodyIndices(forceIndex + i + 0) = pair.first;
        bodyIndices(forceIndex + i + 1) = pair.second;
        i += 2;
    }
}

void Solver::initColliderRow(uint row, uint manifoldIndex, ColliderRow& colliderRow) {
    colliderRow.pos = {getPos()(row, 0), getPos()(row, 1)};
    colliderRow.mat = getMat(row);
    colliderRow.imat = getIMat(row);
    colliderRow.start = getStartPtr(row);
    colliderRow.length = getLength(row);
    colliderRow.index = getIndexA().data() + manifoldIndex * getIndexA().shape(1);
}

void Solver::draw() {
    // TODO draw everything
}