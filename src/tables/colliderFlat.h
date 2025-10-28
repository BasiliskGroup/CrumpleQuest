#ifndef COLLIDER_FLAT_H
#define COLLIDER_FLAT_H

#include "util/includes.h"
#include "tables/virtualTable.h"
#include "tables/eraseChunks.h"

class ColliderFlat {
private:
    uint vertSize = 0;
    uint vertCapacity;
    uint colliderSize = 0;
    uint colliderCapacity;

    std::unordered_map<uint, Collider*> colliders;
    std::vector<uint> toDelete;

    // store vertex data
    std::vector<vec2> verts;
    std::vector<uint> start;
    std::vector<uint> length;

    // store properties
    std::vector<vec2> com;
    std::vector<vec2> halfDim; // used for OBBs and BVH AABBs
    std::vector<float> area;
    std::vector<float> moment;

public:
    ColliderFlat(uint vertCapacity, uint colliderCapacity);
    ~ColliderFlat() = default;

    std::vector<vec2>& getVerts() { return verts; }
    std::vector<uint>& getStart() { return start; }
    std::vector<uint>& getLength() { return length; }

    uint getStart(uint index) { return start[index]; }
    vec2* getStartPtr(uint index) { return verts.data() + start[index]; }
    uint getLength(uint index) { return length[index]; }

    void compact();
    void resize(uint newCapacity); // TODO make a resize function for colliderSoA
    void refreshPointers();
    uint insert(std::vector<vec2> verts);
    void remove(uint colliderIndex);
};

#endif 