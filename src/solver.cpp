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
    uint insertIndex = forceSoA->getManifoldSoA()->reserve(collisionPairs.size());

    int count = 0;

    for (const auto& pair : collisionPairs) {
        uint rowA = pair.first;
        uint rowB = pair.second;

        // create collider rows for better caching
        ColliderRow a = {
            getPos()(rowA, 0), getPos()(rowA, 1),
            getMat()(rowA), getIMat()(rowA),
            getStartPtr(rowA), getLength(rowA),
            xt::view(getIndexA(), insertIndex, xt::all())
        };

        ColliderRow b = {
            getPos()(rowB, 0), getPos()(rowB, 1),
            getMat()(rowB), getIMat()(rowB),
            getStartPtr(rowB), getLength(rowB),
            xt::view(getIndexB(), insertIndex, xt::all())
        };

        auto minks = xt::view(getMinks(), insertIndex, xt::all(), xt::all());
        CollisionPair collisionPair = { insertIndex, {vec2{minks(0, 0), minks(0, 1)}, vec2{minks(1, 0), minks(1, 1)}, vec2{minks(2, 0), minks(2, 1)}}};

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            insertIndex++;
            continue;
        }

        // determine collision normal
        count++;
        ushort frontIndex = epa(a, b, collisionPair);
        print(collisionPair.polytope[frontIndex].normal);

        // determine object overlap

        // increment enumeration
        insertIndex++;
    }

    std::cout << "Narrow: " << count << std::endl;
}

bool Solver::gjk(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex) {
    for (uint _ = 0; _ < GJK_ITERATIONS; ++_) {
        // get next direction or test simplex if full
        freeIndex = handleSimplex(a, b, pair, freeIndex);

        // termination signal
        if (freeIndex == -1) {
            return true;
        }

        // get next support point
        addSupport(a, b, pair, freeIndex);

        // if the point we found didn't cross the origin, we are not colliding
        if (glm::dot(pair.minks[freeIndex], pair.dir) < COLLISION_MARGIN) {
            std::cout << "no cross " << freeIndex << std::endl;
            return false;
        }

        freeIndex++;
    }

    std::cout << "time out" << std::endl;
    return false;
}

uint Solver::handleSimplex(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex) {
    switch (freeIndex) {
        case 0: return handle0(a, b, pair);
        case 1: return handle1(a, b, pair);
        case 2: return handle2(a, b, pair);
        case 3: return handle3(a, b, pair);
        default: throw std::runtime_error("simplex has incorrect freeIndex");
    }
}

uint Solver::handle0(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    pair.dir = b.pos - a.pos;

    // if center of masses are super close, they must be colliding
    // this also avoids numerical stability issues
    if (glm::length2(pair.dir) < COLLISION_MARGIN) {
        return -1;
    }

    return 0;
}

uint Solver::handle1(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    pair.dir = -pair.minks[0];
    return 1;
}

uint Solver::handle2(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    vec2 CB = pair.minks[1] - pair.minks[0];
    vec2 CO =               - pair.minks[0];
    tripleProduct(CB, CO, CB, pair.dir);

    if (glm::length2(pair.dir) < COLLISION_MARGIN) {
        // fallback perpendicular
        perpTowards(CB, CO, pair.dir);
    }

    return 2;
}

uint Solver::handle3(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
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
        return 2;
    }

    vec2 BO = -pair.minks[1];
    perpTowards(AC, BO, perp);
    if (glm::dot(perp, AO) > -COLLISION_MARGIN) {
        // remove 1
        a.index(1)    = a.index(2);
        b.index(1)    = b.index(2);
        pair.minks[1] = pair.minks[2];

        pair.dir = perp;
        return 2;
    }

    // we have found a collision
    return -1;
}

void Solver::addSupport(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex) {
    // direct search vector into local space
    vec2 dirA = a.imat *  pair.dir;
    vec2 dirB = b.imat * (-pair.dir);

    a.index[insertIndex] = getFar(a.start, a.length, dirA);
    b.index[insertIndex] = getFar(b.start, b.length, dirB);

    // Transform selected local vertices into world space
    vec2 localA = a.start[a.index[insertIndex]];
    vec2 localB = b.start[b.index[insertIndex]];
    vec2 worldA = a.pos + a.mat * localA;
    vec2 worldB = b.pos + b.mat * localB;

    // Compute Minkowski support point
    pair.minks[insertIndex] = worldA - worldB;
}

// TODO find the error in this hill climbing, not 100% accurate
// uint Solver::getFar(const vec2* verts, uint length, const vec2& dir) {
//     uint cur = 0;
//     float here = glm::dot(dir, verts[0]);

//     // select search direction
//     uint end = length - 1;
//     float roll = glm::dot(dir, verts[end]);
//     float right = glm::dot(dir, verts[1]);

//     // early out if the first index is at the top of the hill
//     if (here > roll && here > right) {
//         return cur;
//     }
    
//     // prepare info for walk direction
//     uint walk;
//     if (roll > right) {
//         walk = -1;
//         cur = end;
//         here = roll;
//     } else {
//         walk = 1;
//         cur = 1;
//         here = right;
//     }

//     // walk until we find a worse vertex
//     uint nextIdx, nextDot;
//     while (0 <= cur && cur <= end) {
//         nextIdx = cur + walk;
//         if (0 > nextIdx || nextIdx > end) {
//             return cur; // we have hit the boundary and should leave
//         }
//         nextDot = glm::dot(dir, verts[nextIdx]);
//         if (nextDot < here) {
//             return cur;
//         }
//         cur = nextIdx;
//         here = nextDot;
//     }

//     return cur;
// }

uint Solver::getFar(const vec2* verts, uint length, const vec2& dir) {
    uint farIndex = 0;
    float maxDot = glm::dot(verts[0], dir);

    for (uint i = 1; i < length; ++i) {
        float d = glm::dot(verts[i], dir);
        if (d > maxDot) {
            maxDot = d;
            farIndex = i;
        }
    }

    return farIndex;
}

ushort Solver::epa(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    // deepcopy minkowski points into support points list
    std::copy(pair.minks.begin(), pair.minks.end(), pair.sps.begin());

    buildFace(pair, 0, 1, 0);
    buildFace(pair, 1, 2, 1);
    buildFace(pair, 2, 0, 2);

    ushort cloudSize = 3;
    ushort numFaces = 3;
    ushort setSize = 0;

    ushort frontIndex;
    float frontDistance;
    for (ushort _ = 0; _ < EPA_ITERATIONS; _++) {
        // quick access front data
        frontIndex = polytopeFront(pair.polytope, numFaces);
        frontDistance = pair.polytope[frontIndex].distance;

        pair.dir = pair.polytope[frontIndex].normal;
        supportSpOnly(a, b, pair, cloudSize);

        // check if newly added point is not in the cloud
        // if so, we have found the edge and can stop
        for (ushort i = 0; i < cloudSize; i++) {
            if (glm::length2(pair.sps[cloudSize] - pair.sps[i]) < COLLISION_MARGIN * COLLISION_MARGIN) {
                print("cloud out");
                return frontIndex; 
            }
        }

        // check that the new found point is past the face
        // this is to ensure that the new point has expanded the polytope
        if (glm::dot(pair.polytope[frontIndex].normal, pair.sps[cloudSize]) - frontDistance < COLLISION_MARGIN * COLLISION_MARGIN) {
            print("tolerance out");
            return frontIndex;
        }

        // collect horizon edges
        setSize = 0;
        ushort i = 0;
        while (i < numFaces) {
            if (glm::dot(pair.polytope[i].normal, pair.sps[cloudSize]) > 0) {
                setSize = insertHorizon(pair.spSet, pair.polytope[i].va, setSize);
                setSize = insertHorizon(pair.spSet, pair.polytope[i].va, setSize);
                removeFace(pair.polytope, i, numFaces);
                numFaces -= 1;
            } else {
                i++;
            }
        }

        // there should be only 2 horizon vertices left, create new face
        buildFace(pair, pair.spSet[0], cloudSize, numFaces);
        buildFace(pair, cloudSize, pair.spSet[1], numFaces);
        numFaces += 2;

        // we increment cloud size at the end to reduce subtractions in the middle of the loop
        cloudSize++;
    }

    // we timed out
    print("time out");
    return polytopeFront(pair.polytope, numFaces);
}

/**
 * @brief inserts the horizon index into the set if it does not already exist. If it does, remove the existing index. 
 * 
 * @param spSet 
 * @param spIndex 
 * @param setSize 
 * @return ushort 
 */
ushort Solver::insertHorizon(SpSet& spSet, ushort spIndex, ushort setSize) {
    if (discardHorizon(spSet, spIndex, setSize)) {
        return --setSize;
    }
    spSet[setSize] = spIndex;
    return ++setSize;
}

/**
 * @brief Swaps search index to back of set if it exists. Returns whether or not the index was found.
 * 
 * @param spSet 
 * @param spIndex 
 * @param setSize 
 * @return true 
 * @return false 
 */
bool Solver::discardHorizon(SpSet& spSet, ushort spIndex, ushort setSize) {
    if (setSize == 0) {
        return false;
    }

    // uses setSize - 1 since swapping last index wouldn't move it
    for (short i = 0; i < setSize - 1; i++) {
        if (spSet[i] == spIndex) {
            spSet[i] = spSet[setSize - 1];
        }
    }

    // no need to swap last but we still need to check if it should be removed
    return spSet[setSize - 1] == spIndex;
}

/**
 * @brief Finds the index of the polytope face that is closest to the origin. Runs a simple linear search for minimum overhead cost. 
 * 
 * @param polytope 
 * @param setSize . k > 2
 * @return ushort 
 */
ushort Solver::polytopeFront(const Polytope& polytope, ushort numFaces) {
    ushort closeIndex = 0;
    float closeValue = polytope[0].distance;
    float value;

    for (ushort i = 1; i < numFaces; i++) {
        value = polytope[i].distance;
        if (value < closeValue) {
            closeValue = value;
            closeIndex = i;
        }
    }

    return closeIndex;
}

/**
 * @brief swap and pop the given face out of polytope
 * 
 * @param polytope 
 * @param index 
 * @param size 
 */
void Solver::removeFace(Polytope& polytope, ushort index, ushort numFaces) {
    polytope[index] = polytope[numFaces - 1];
}

/**
 * @brief inserts a support point directly into the sp array without saving witness indices. 
 * 
 * @param a 
 * @param b 
 * @param pair 
 * @param insertIndex 
 */
void Solver::supportSpOnly(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex) {
    // direct search vector into local space
    vec2 dirA = a.imat *  pair.dir;
    vec2 dirB = b.imat * (-pair.dir);

    // Transform selected local vertices into world space
    vec2 localA = a.start[getFar(a.start, a.length, dirA)]; // TODO replace these with a transform function
    vec2 localB = b.start[getFar(b.start, b.length, dirB)];
    vec2 worldA = a.pos + a.mat * localA;
    vec2 worldB = b.pos + b.mat * localB;

    // Compute Minkowski support point
    pair.sps[insertIndex] = worldA - worldB;
}

/**
 * @brief Create polyotpe face from vertices at A and B and log it in the polytope at L.
 * 
 * @param pair 
 * @param indexA 
 * @param indexB 
 * @param indexL 
 */
void Solver::buildFace(CollisionPair& pair, ushort indexA, ushort indexB, ushort indexL) {
    Polytope& polytope = pair.polytope;
    PolytopeFace& face = polytope[indexL];
    face.va = indexA;
    face.vb = indexB;

    vec2 edge = pair.sps[indexB] - pair.sps[indexA];

    // TODO check if we need midpoint or could just use vertex a
    vec2 mid = pair.sps[indexA] + 0.5f * edge;
    vec2 normal = { -edge.y, edge.x };
    if (glm::dot(mid, normal) < 0) {
        normal *= -1.0f;
    }

    face.normal = glm::normalize(normal);
    face.distance = glm::dot(normal, pair.sps[indexA]);
}

void Solver::sat() {
    
}

void Solver::draw() {
    // TODO draw everything
}