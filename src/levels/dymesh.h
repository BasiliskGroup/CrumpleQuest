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

    DyMesh(const std::vector<Point64>& region, Mesh* mesh);
    DyMesh(const std::vector<Point64>& region, const std::vector<Tri>& data);
    DyMesh(const std::vector<Point64>& region);

    // modifier functions
    void cut(const std::vector<Point64>& clipRegion); // delete region
    void cut(const DyMesh& other); // delete region
    bool copy(const DyMesh& other); // copy all uvs from containing shape
    bool copyIntersection(const DyMesh& other); // copy region and UVs from intersection
    void paste(const DyMesh& other); // paste incoming shape into us

    // collision checks
    bool hasOverlap(const DyMesh& other) const;
    bool contains(const vec2& pos) const;
    
    // Mirror using two-point line representation (updated signature)
    DyMesh* mirror(const Point64& p0, const Point64& p1);

    bool sampleUV(const vec2& v, vec2& uv) const;
    bool sampleUV(const Vert& v, vec2& uv) const { return sampleUV(v.pos, uv); }

    int getTrindex(const vec2& pos) const;

    // export
    void toData(std::vector<float>& exp);
    void printData();
};

#endif