#include "levels/triangle.h"

bool Tri::contains(const vec2& pos) const {
    // TODO check if these are wound in the correct direction for the screen
    float d1 = sign(pos, verts[0], verts[1]); 
    float d2 = sign(pos, verts[1], verts[2]);
    float d3 = sign(pos, verts[2], verts[0]);

    bool hasNeg = d1 < 0 || d2 < 0 || d3 < 0;
    bool hasPos = d1 > 0 || d2 > 0 || d3 > 0;

    return !(hasNeg && hasPos);
}

vec2 Tri::intersect(const vec2& pos, const vec2& dir) {
    float closestT = std::numeric_limits<float>::max();
    vec2 closestPoint = pos;

    for (int i = 0; i < 3; ++i) {
        const vec2& a = verts[i];
        const vec2& b = verts[(i + 1) % 3];
        vec2 hit;

        if (intersectLineSegmentInfiniteLine(a, b, pos, dir, hit)) {
            float t = glm::dot(hit - pos, dir);
            if (t > 0 && t < closestT) {
                closestT = t;
                closestPoint = hit;
            }
        }
    }

    return closestPoint;
}