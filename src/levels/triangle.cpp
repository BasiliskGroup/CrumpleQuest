#include "levels/triangle.h"

Tri::Tri(std::array<vec2, 3> verts) {
    for (uint i = 0; i < 3; i++) {
        this->verts[i] = Vert(verts[i], {});
    }
}

float Tri::distance(const vec2& pos) const {
    // Find the closest point on the triangle to pos
    vec2 closestPoint = closestPointOnTriangle(pos);
    return glm::distance(pos, closestPoint);
}

vec2 Tri::closestPointOnTriangle(const vec2& pos) const {
    const vec2& a = verts[0].pos;
    const vec2& b = verts[1].pos;
    const vec2& c = verts[2].pos;

    // Check if point is inside triangle
    auto sign = [](const vec2& p1, const vec2& p2, const vec2& p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    };

    float d1 = sign(pos, a, b);
    float d2 = sign(pos, b, c);
    float d3 = sign(pos, c, a);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // If inside triangle, return the point itself
    if (!(hasNeg && hasPos)) {
        return pos;
    }

    // Point is outside, find closest point on edges
    auto closestPointOnSegment = [](const vec2& p, const vec2& a, const vec2& b) {
        vec2 ab = b - a;
        float t = glm::dot(p - a, ab) / glm::dot(ab, ab);
        t = glm::clamp(t, 0.0f, 1.0f);
        return a + t * ab;
    };

    vec2 p1 = closestPointOnSegment(pos, a, b);
    vec2 p2 = closestPointOnSegment(pos, b, c);
    vec2 p3 = closestPointOnSegment(pos, c, a);

    float dist1 = glm::distance(pos, p1);
    float dist2 = glm::distance(pos, p2);
    float dist3 = glm::distance(pos, p3);

    if (dist1 <= dist2 && dist1 <= dist3) return p1;
    if (dist2 <= dist3) return p2;
    return p3;
}

vec2 Tri::intersect(const vec2& pos, const vec2& dir) {
    float closestT = std::numeric_limits<float>::max();
    vec2 closestPoint = pos;

    for (int i = 0; i < 3; ++i) {
        const vec2& a = verts[i].pos;
        const vec2& b = verts[(i + 1) % 3].pos;
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

vec2 Tri::leastDot(const vec2& dir) {
    float d0 = glm::dot(verts[0].pos, dir);
    float d1 = glm::dot(verts[1].pos, dir);
    float d2 = glm::dot(verts[2].pos, dir);

    if (d0 < d1 && d0 < d2) return verts[0].pos;
    if (d1 < d0 && d1 < d2) return verts[1].pos;
    return verts[2].pos;
}

// ----------------------------
// Updated function: sampleUV()
// Now samples UV at the nearest point on the triangle
// ----------------------------
vec2 Tri::sampleUV(const vec2& pos) const {
    // First find the nearest point on the triangle
    vec2 nearestPoint = closestPointOnTriangle(pos);

    const vec2& a = verts[0].pos;
    const vec2& b = verts[1].pos;
    const vec2& c = verts[2].pos;

    float denom = (b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y);
    const float eps = 1e-8f;
    
    if (fabs(denom) < eps) {
        // Degenerate triangle: return nearest vertex UV
        float da = glm::distance(nearestPoint, a);
        float db = glm::distance(nearestPoint, b);
        float dc = glm::distance(nearestPoint, c);
        if (da <= db && da <= dc) return verts[0].uv;
        if (db <= da && db <= dc) return verts[1].uv;
        return verts[2].uv;
    }

    // Calculate barycentric coordinates for the nearest point
    float w1 = ((b.y - c.y)*(nearestPoint.x - c.x) + (c.x - b.x)*(nearestPoint.y - c.y)) / denom;
    float w2 = ((c.y - a.y)*(nearestPoint.x - c.x) + (a.x - c.x)*(nearestPoint.y - c.y)) / denom;
    float w3 = 1.0f - w1 - w2;

    // Clamp weights to [0,1]
    w1 = glm::clamp(w1, 0.0f, 1.0f);
    w2 = glm::clamp(w2, 0.0f, 1.0f);
    w3 = glm::clamp(w3, 0.0f, 1.0f);

    vec2 uv = verts[0].uv * w1 + verts[1].uv * w2 + verts[2].uv * w3;
    return uv;
}

void Tri::print() const {
    std::cout << "| ";
    for (const Vert& vert : verts) std::cout << vert.pos.x << " " << vert.pos.y << " " << vert.uv.x << " " << vert.uv.y << " | ";
    std::cout << std::endl;
}

void Tri::flipUVx() {
    // Extract current UV.x values
    float u0 = verts[0].uv.x;
    float u1 = verts[1].uv.x;
    float u2 = verts[2].uv.x;

    // Compute min and max range
    float minU = std::min({u0, u1, u2});
    float maxU = std::max({u0, u1, u2});

    float range = maxU - minU;
    if (range < 1e-12f) return; // Avoid degenerate case

    // Flip each UV.x within the triangle's local range
    for (Vert& v : verts) {
        float oldU = v.uv.x;
        float newU = maxU - (oldU - minU);  // reflection inside [minU, maxU]
        v.uv.x = newU;
    }
}
