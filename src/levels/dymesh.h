#ifndef DYMESH_H
#define DYMESH_H

#include "util/includes.h"
#include "util/maths.h"

#include "levels/triangle.h"
#include "levels/edger.h"

#include <earcut.hpp> // mapbox earcut imported through assimp
#include "util/clipper_helper.h"

struct DyMesh : public Edger {
    std::vector<Tri> data; // the current state of the mesh

    DyMesh(const std::vector<vec2>& region, Mesh* mesh);
    DyMesh(const std::vector<vec2>& region, const std::vector<Tri>& data);
    DyMesh(const std::vector<vec2>& region); // NOTE probably temporary, 

    // modifier functions
    bool cut(const std::vector<vec2>& clipRegion, bool useIntersection = false); // delete region
    bool cut(const DyMesh& other, bool useIntersection = false); // delete region
    bool copy(const DyMesh& other); // copy all uvs from containing shape
    bool paste(const DyMesh& other, int expected = -1); // paste incoming shape intop of us

    // collision checks
    bool hasOverlap(const DyMesh& other) const;
    bool contains(const vec2& pos) const;
    int getTrindex(const vec2& pos) const;
    bool sampleUV(const vec2& v, vec2& uv) const;
    bool sampleUV(const Vert& v, vec2& uv) const { return sampleUV(v.pos, uv); }
    
    DyMesh* mirror(const vec2& pos, const vec2& dir);
    void flipHorizontal();

    // export
    void toData(std::vector<float>& exp);
    void printData();

    // cleaning
    void removeDataOutside();
};

#endif
