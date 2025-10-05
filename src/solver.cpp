#include "physics.h"

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
    bodySoA->compact(); // compact bodies to increase cache hits
    bodySoA->computeTransforms();

    // NOTE bodies are compact after this point

    sphericalCollision(); // broad collision TODO replace with BVH
    std::cout << "Broad: " << collisionPairs.size() << std::endl;
    narrowCollision(); // -> uncompacts manifolds
    // warmstart forces -> uncompacts forces
    forceSoA->compact();

    // NOTE bodies and forces are compact after this point

}

// TODO replace this with BVH
void Solver::sphericalCollision() {
    // clear contact pairs from last frame
    collisionPairs.clear();

    // generate new contact pairs
    uint num_bodies = bodySoA->getSize();
    auto pos = bodySoA->getPos();
    auto radii = bodySoA->getRadius();

    // collect difference in positions
    auto diff = xt::view(pos, xt::all(), xt::newaxis(), xt::all()) - xt::view(pos, xt::newaxis(), xt::all(), xt::all());
    auto dist2 = xt::sum(xt::pow(diff, 2), {2});

    // collect radii
    auto rsum = xt::view(radii, xt::all(), xt::newaxis()) + xt::view(radii, xt::newaxis(), xt::all());
    auto rsum2 = xt::pow(rsum, 2);

    // find differences
    auto table = rsum2 - dist2;

    // load collision pairs
    for (uint i = 0; i < num_bodies; i++) {
        for (uint j = i + 1; j < num_bodies; j++) {
            if (table(i, j) > 0.0) {
                collisionPairs.emplace_back(i, j);
            }
        }
    }
}

void Solver::narrowCollision() {
    // reserve space to perform all collisions
    uint insertIndex = forceSoA->getManifoldSoA()->reserve(collisionPairs.size());

    int count = 0;

    for (const auto& pair : collisionPairs) {
        uint rowA = pair.first;
        uint rowB = pair.second;

        // create collider rows for better caching
        auto imatTemp = xt::view(getIMat(), rowA, xt::all(), xt::all());
        ColliderRow a = {
            getPos()(rowA, 0), getPos()(rowA, 1),
            { imatTemp(0, 0), imatTemp(0, 1), imatTemp(1, 0), imatTemp(1, 1) },
            getStartPtr(rowA), getLength(rowA),
            xt::view(getIndexA(), insertIndex, xt::all())
        };

        imatTemp = xt::view(getIMat(), rowB, xt::all(), xt::all());
        ColliderRow b = {
            getPos()(rowB, 0), getPos()(rowB, 1),
            { imatTemp(0, 0), imatTemp(0, 1), imatTemp(1, 0), imatTemp(1, 1) },
            getStartPtr(rowB), getLength(rowB),
            xt::view(getIndexB(), insertIndex, xt::all())
        };

        auto minks = xt::view(getMinks(), insertIndex, xt::all(), xt::all());
        CollisionPair collisionPair = { insertIndex, {{minks(0, 0), minks(0, 1)}, {minks(1, 0), minks(1, 1)}, {minks(2, 0), minks(2, 1)}}, 0 };

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair);

        if (!collided) {
            // increment enumeration
            insertIndex++;
            continue;
        }

        count++;

        // determine collision normal

        // determine object overlap

        // increment enumeration
        insertIndex++;
    }

    std::cout << "Narrow: " << count << std::endl;
}

bool Solver::gjk(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    for (uint _ = 0; _ < GJK_ITERATIONS; ++_) {
        // get next direction or test simplex if full
        handleSimplex(a, b, pair);

        // termination signal
        if (pair.freeIndex == -1) {
            return true;
        }

        // get next support point
        addSupport(a, b, pair);

        // if the point we found didn't cross the origin, we are not colliding
        if (glm::dot(pair.minks[pair.freeIndex], pair.dir) < COLLISION_MARGIN) {
            std::cout << "no cross: " << pair.freeIndex << std::endl;
            return false;
        }

        pair.minks[pair.freeIndex]++;
    }

    std::cout << "time out" << std::endl;
    return false;
}

void Solver::handleSimplex(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    switch (pair.freeIndex) {
        case 0: return handle0(a, b, pair);
        case 1: return handle1(a, b, pair);
        case 2: return handle2(a, b, pair);
        case 3: return handle3(a, b, pair);
        default: throw std::runtime_error("simplex has incorrect freeIndex");
    }
}

void Solver::handle0(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    pair.dir = b.pos - a.pos;

    // if center of masses are super close, they must be colliding
    // this also avoids numerical stability issues
    if (glm::length2(pair.dir) < COLLISION_MARGIN) {
        pair.freeIndex = -1;
        return;
    }

    pair.freeIndex = 0;
}

void Solver::handle1(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    pair.dir = -pair.minks[0];
    pair.freeIndex = 1;
}

void Solver::handle2(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    vec2 CB = pair.minks[1] - pair.minks[2];
    vec2 CO =               - pair.minks[2];

    // TODO check for an extra robustness case (may not be needed)
    tripleProduct(CB, CO, CB, pair.dir);
    pair.freeIndex = 2;
}

void Solver::handle3(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    vec2 AB = pair.minks[1] - pair.minks[2];
    vec2 AC = pair.minks[0] - pair.minks[2];
    vec2 AO =               - pair.minks[2];
    vec2 CO =               - pair.minks[0];

    vec2 perp;
    perpTowards(AB, CO, perp);
    if (glm::dot(perp, AO) > -COLLISION_MARGIN) {
        // remove 0
        a.index(0)    = a.index(2);
        b.index(0)    = b.index(2);
        pair.minks[0] = pair.minks[2];

        pair.dir = perp;
        pair.freeIndex = 2;
        return;
    }

    vec2 BO = -pair.minks[1];
    perpTowards(AC, BO, perp);
    if (glm::dot(perp, AO) > -COLLISION_MARGIN) {
        // remove 1
        a.index(0)    = a.index(2);
        b.index(0)    = b.index(2);
        pair.minks[0] = pair.minks[2];

        pair.dir = perp;
        pair.freeIndex = 2;
        return;
    }

    // we have found a collision
    pair.freeIndex = -1;
}

/**
 * @brief Finds the index of the vertex with the highest dot product with dir. Uses hill climbing for optimization over vectorization. 
 * 
 * @param rigid 
 * @param dir 
 * @return uint
 */
uint Solver::getFar(vec2* verts, uint length, vec2& dir) {
    uint cur = 0;
    float here = glm::dot(dir, verts[0]);

    // select search direction
    uint end = length - 1;
    float roll = glm::dot(dir, verts[end]);
    float right = glm::dot(dir, verts[1]);

    // early out if the first index is at the top of the hill
    if (here > roll && here > right) {
        return cur;
    }
    
    // prepare info for walk direction
    uint walk;
    if (roll > right) {
        walk = -1;
        cur = end;
        here = roll;
    } else {
        walk = 1;
        cur = 1;
        here = right;
    }

    // walk until we find a worse vertex
    uint nextIdx, nextDot;
    while (0 <= cur && cur <= end) {
        nextIdx = cur + walk;
        if (0 > nextIdx || nextIdx > end) {
            return cur; // we have hit the boundary and should leave
        }
        nextDot = glm::dot(dir, verts[nextIdx]);
        if (nextDot < here) {
            return cur;
        }
        cur = nextIdx;
        here = nextDot;
    }

    return cur;
}

void Solver::addSupport(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    // direct search vector into local space
    vec2 dirA = a.imat *  pair.dir;
    vec2 dirB = b.imat * -pair.dir;

    a.index[pair.freeIndex] = getFar(a.start, a.length, dirA);
    b.index[pair.freeIndex] = getFar(b.start, b.length, dirB);
}

void Solver::epa() {

}

void Solver::sat() {
    
}

void Solver::draw() {
    // TODO draw everything
}