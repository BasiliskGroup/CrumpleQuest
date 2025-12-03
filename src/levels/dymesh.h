#ifndef DYMESH_H
#define DYMESH_H

#include "util/includes.h"
#include "util/maths.h"
#include "levels/uvregion.h"
#include "levels/edger.h"
#include <earcut.hpp>
#include "util/clipper_helper.h"

struct DyMesh : public Edger {
    std::vector<UVRegion> regions;  // Independent UV regions that compose the mesh

    DyMesh(const std::vector<vec2>& region, Mesh* mesh);
    DyMesh(const std::vector<vec2>& region, const std::vector<UVRegion>& regions);
    DyMesh(const std::vector<vec2>& region);  // Create single region with default UVs

    // Modifier functions
    bool cut(const std::vector<vec2>& clipRegion, bool useIntersection = false);
    bool cut(const DyMesh& other, bool useIntersection = false);
    bool copy(const DyMesh& other);  // Copy UVs from overlapping regions
    bool paste(const DyMesh& other, int expected = -1);  // Paste aligned mesh

    // Collision checks
    bool hasOverlap(const DyMesh& other) const;
    bool contains(const vec2& pos) const;
    bool sampleUV(const vec2& pos, vec2& uv) const;
    bool sampleUV(const Vert& v, vec2& uv) const { return sampleUV(v.pos, uv); }
    
    DyMesh* mirror(const vec2& pos, const vec2& dir);
    void flipHorizontal();

    // Export
    void toData(std::vector<float>& exp);
    void printData();

    // Cleaning
    void removeDataOutside();
    void mergeAllRegions();

private:
    bool canMergeRegions(const UVRegion& r1, const UVRegion& r2, std::vector<vec2>& sharedEdge) const;
    bool hasSharedEdge(const UVRegion& r1, const UVRegion& r2, std::vector<vec2>& sharedEdge) const;
    bool hasCompatibleUVs(const UVRegion& r1, const UVRegion& r2, const std::vector<vec2>& sharedEdge) const;
    UVRegion mergeTwo(const UVRegion& r1, const UVRegion& r2) const;
    void cleanupDegenerateRegions();
    // void removeContainedRegions();
};

#endif