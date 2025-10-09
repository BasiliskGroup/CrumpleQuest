#include "solver/physics.h"

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
            return false;
        }

        freeIndex++;
    }
    
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
        a.index[0]    = a.index[2];
        b.index[0]    = b.index[2];
        pair.minks[0] = pair.minks[2];

        pair.dir = perp;
        return 2;
    }

    vec2 BO = -pair.minks[1];
    perpTowards(AC, BO, perp);
    if (glm::dot(perp, AO) > -COLLISION_MARGIN) {
        // remove 1
        a.index[1]    = a.index[2];
        b.index[1]    = b.index[2];
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