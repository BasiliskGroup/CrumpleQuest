#ifndef DYMESH_H
#define DYMESH_H

#include "util/includes.h"
#include "levels/triangle.h"

#include <earcut.hpp> // mapbox earcut imported through assimp
#include "clipper2/clipper.h"

struct DyMesh {
    std::vector<vec2> region; // all vertices that make the perimeter of the shape, wound CCW
    std::vector<Tri> data; // the current state of the mesh

    // constructors, Mesh*, vector of vec2, Tri
    DyMesh(Mesh* mesh);
    DyMesh(const std::vector<vec2>& region, const std::vector<Tri>& data);

    // geometry operations (implemented in dymesh.cpp)
    void cut(const std::vector<vec2>& clipRegion);
    void cut(const DyMesh& other);
    void copy(const DyMesh& other);
    void paste(const DyMesh& other);

    // sample UV
    vec2 sampleUV(const vec2& pos) const;
    vec2 sampleUV(const Vert& v) const { return sampleUV(v.pos); }

    // export
    void toData(std::vector<float>& data);
};

#endif
