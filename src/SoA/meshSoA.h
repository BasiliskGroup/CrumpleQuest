#ifndef MESHSOA_H
#define MESHSOA_H

#include "util/includes.h"
#include "util/indexed.h"
#include "SoA/helper.h"

class MeshSoA {
private:
    std::unordered_map<uint, Indexed*> meshes;
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
    MeshSoA(uint vertsCapacity, uint meshCapacity);
    ~MeshSoA() = default;

    std::vector<vec2>& getVerts() { return verts; }
    std::vector<uint>& getStart() { return start; }
    std::vector<uint>& getLength() { return length; }

    uint getStart(uint index) { return start[index]; }
    vec2* getStartPtr(uint index) { return verts.data() + start[index]; }
    uint getLength(uint index) { return length[index]; }

    void compact();
    void refreshPointers();
    uint insert(std::vector<vec2> verts);
    void remove(uint meshIndex);
};

#endif 