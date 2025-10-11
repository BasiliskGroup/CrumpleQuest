#include "solver/physics.h"

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
                return frontIndex; 
            }
        }

        // check that the new found point is past the face
        // this is to ensure that the new point has expanded the polytope
        if (glm::dot(pair.polytope[frontIndex].normal, pair.sps[cloudSize]) - frontDistance < COLLISION_MARGIN * COLLISION_MARGIN) {
            return frontIndex;
        }

        // collect horizon edges
        setSize = 0;
        ushort i = 0;
        // print("NUM FACES");
        // print(numFaces);
        while (i < numFaces) {
            if (glm::dot(pair.polytope[i].normal, pair.sps[cloudSize]) > COLLISION_MARGIN * COLLISION_MARGIN) {
                // print("adding face");
                // print(pair.polytope[i].va);
                // print(pair.polytope[i].vb);

                setSize = insertHorizon(pair.spSet, pair.polytope[i].va, setSize);
                setSize = insertHorizon(pair.spSet, pair.polytope[i].vb, setSize);
                removeFace(pair.polytope, i, numFaces);
                numFaces -= 1;

                // print("SpSet after add face");
                // for (ushort i = 0; i < setSize; i++) {
                //     print(pair.spSet[i]);
                // }
            } else {
                i++;
            }
        }



        // print("SpSet");
        // for (ushort i = 0; i < setSize; i++) {
        //     print(pair.spSet[i]);
        // }

        if (setSize != 2) {
            print("Polytope horizon error");
            return polytopeFront(pair.polytope, numFaces);
            // throw std::runtime_error("EPA could not find horizon vertices");
        }

        // there should be only 2 horizon vertices left, create new face
        buildFace(pair, pair.spSet[0], cloudSize, numFaces);
        buildFace(pair, cloudSize, pair.spSet[1], numFaces + 1);
        numFaces += 2;

        // we increment cloud size at the end to reduce subtractions in the middle of the loop
        cloudSize++;
    }

    // we timed out
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
    // print("SpSet before discard");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }

    if (discardHorizon(spSet, spIndex, setSize)) {
        // print("SpSet before discarded");
        // for (ushort i = 0; i < setSize; i++) {
        //     print(spSet[i]);
        // }
        return --setSize;
    }

    spSet[setSize] = spIndex;
    setSize++;
    // print("SpSet after add point");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }
    return setSize;
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
    // print("SpSet enter discard");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }
    if (setSize == 0) {
        return false;
    }

    // uses setSize - 1 since swapping last index wouldn't move it
    for (ushort i = 0; i < setSize - 1; i++) {
        if (spSet[i] == spIndex) {
            std::swap(spSet[i], spSet[setSize - 1]);
            break;
        }
    }

    // print("SpSet in discard");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }

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
    vec2 localA; // TODO replace these with a transform function
    getFar(a.start, a.length, dirA, localA);
    vec2 localB;
    getFar(b.start, b.length, dirB, localB);
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

    // print("face indices");
    // print(indexB);
    // print(indexA);

    vec2 edge = pair.sps[indexB] - pair.sps[indexA];

    // TODO check if we need midpoint or could just use vertex a
    vec2 normal = { -edge.y, edge.x };
    if (glm::dot(pair.sps[indexA], normal) < 0) {
        normal *= -1.0f;
    }

    face.normal = glm::normalize(normal);
    face.distance = glm::dot(face.normal, pair.sps[indexA]);
}