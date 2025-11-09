#include "edger.h"

Edger::Edger(const std::vector<vec2> verts) : verts(verts) {}

vec2 Edger::getNearestEdgeIntersection(const vec2& pos, const vec2& dir) {
    float closestT = std::numeric_limits<float>::max();
    vec2 closestPoint = pos;

    size_t n = verts.size();
    if (n < 2)
        return pos; // no edges

    for (size_t i = 0; i < n; ++i) {
        const vec2& a = verts[i];
        const vec2& b = verts[(i + 1) % n];
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

vec2 Edger::getNearestEdgePoint(const vec2& pos) {
    size_t n = verts.size();
    if (n == 0) return pos;
    if (n == 1) return verts[0];

    float bestDistSq = std::numeric_limits<float>::max();
    vec2 bestPoint = pos;

    for (size_t i = 0; i < n; ++i) {
        const vec2& a = verts[i];
        const vec2& b = verts[(i + 1) % n];
        vec2 candidate = nearestPointOnEdgeToPoint(a, b, pos);
        float distSq = glm::dot(candidate - pos, candidate - pos);

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestPoint = candidate;
        }
    }

    return bestPoint;
}