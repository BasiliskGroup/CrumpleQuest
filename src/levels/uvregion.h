#ifndef UVREGION_H
#define UVREGION_H

#include "util/includes.h"
#include "util/maths.h"

struct UVRegion {
    std::vector<vec2> positions;     // Polygon vertices
    std::array<Vert, 2> basis;       // Two linearly independent direction vectors (pos delta, uv delta)
    vec2 originUV;                   // UV at positions[0] (the origin)
    bool isObstacle;

    UVRegion() = default;
    UVRegion(const std::vector<vec2>& positions, const std::array<Vert, 2>& basis, const vec2& originUV, bool isObstacle=false);
    UVRegion(Mesh* mesh, const vec3& position, const vec2& scale, bool isObstacle);

    // Sample UV at any point using linear basis transformation
    vec2 sampleUV(const vec2& pos) const;
    
    // Check if point is inside region (with epsilon tolerance)
    bool contains(const vec2& pos, float eps = 1e-6f) const;
    
    // Get distance from point to region
    float distance(const vec2& pos) const;

    void flipUVx();
    void flipHorizontal();
};

#endif