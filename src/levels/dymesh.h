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

    void cut(const std::vector<vec2>& clipRegion); // delete region
    void cut(const DyMesh& other); // delete region
    void copy(const DyMesh& other); // copy all uvs from containing shape
    void paste(const DyMesh& other); // paste incoming shape intop of us
    void pasteWithin(const DyMesh& other); // paste only sections that are within our region

    vec2 sampleUV(const vec2& pos) const;
    vec2 sampleUV(const Vert& v) const { return sampleUV(v.pos); }

    uint getTrindex(const vec2& pos) const;

    // export
    void toData(std::vector<float>& data);
};

#endif
