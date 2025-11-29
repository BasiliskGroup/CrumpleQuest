#ifndef UVREGION_H
#define UVREGION_H

#include "util/includes.h"
#include "util/maths.h"

struct UVRegion {
    std::vector<vec2> positions;  // Polygon vertices
    std::vector<vec2> uvs;        // UV coordinates (same size as positions)

    UVRegion() = default;
    UVRegion(const std::vector<vec2>& positions, const std::vector<vec2>& uvs);

    // Sample UV at any point using barycentric interpolation
    vec2 sampleUV(const vec2& pos) const;
    
    // Check if point is inside region (with epsilon tolerance)
    bool contains(const vec2& pos, float eps = 1e-6f) const;
    
    // Get distance from point to region
    float distance(const vec2& pos) const;

    void flipUVx();
    void flipHorizontal();
};

#endif