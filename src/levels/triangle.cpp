#include "levels/triangle.h"

Tri::Tri(std::array<vec2, 3> verts) {
    for (uint i = 0; i < 3; i++) {
        this->verts[i] = Vert(verts[i], {});
    }
}

bool Tri::contains(const vec2& pos) const {
    auto sign_eps = [](const vec2& p1, const vec2& p2, const vec2& p3) {
        const float eps = 1e-6f;
        float val = (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        if (fabs(val) < eps) return 0.0f; // Treat as zero if very close
        return val;
    };

    float d1 = sign_eps(pos, verts[0].pos, verts[1].pos);
    float d2 = sign_eps(pos, verts[1].pos, verts[2].pos);
    float d3 = sign_eps(pos, verts[2].pos, verts[0].pos);

    bool hasNeg = d1 < 0 || d2 < 0 || d3 < 0;
    bool hasPos = d1 > 0 || d2 > 0 || d3 > 0;

    return !(hasNeg && hasPos);
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
// New function: sampleUV()
// ----------------------------
vec2 Tri::sampleUV(const vec2& pos) const {
    const vec2& a = verts[0].pos;
    const vec2& b = verts[1].pos;
    const vec2& c = verts[2].pos;

    float denom = (b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y);
    const float eps = 1e-8f;
    
    if (fabs(denom) < eps) {
        // Degenerate triangle: return nearest vertex UV
        float da = glm::distance(pos, a);
        float db = glm::distance(pos, b);
        float dc = glm::distance(pos, c);
        if (da <= db && da <= dc) return verts[0].uv;
        if (db <= da && db <= dc) return verts[1].uv;
        return verts[2].uv;
    }

    float w1 = ((b.y - c.y)*(pos.x - c.x) + (c.x - b.x)*(pos.y - c.y)) / denom;
    float w2 = ((c.y - a.y)*(pos.x - c.x) + (a.x - c.x)*(pos.y - c.y)) / denom;
    float w3 = 1.0f - w1 - w2;

    // Clamp weights to [0,1] to avoid slight negative values from floating point errors
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