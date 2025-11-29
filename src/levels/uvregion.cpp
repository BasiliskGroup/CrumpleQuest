#include "levels/uvregion.h"
#include <earcut.hpp>

UVRegion::UVRegion(const std::vector<vec2>& positions, const std::vector<vec2>& uvs) 
    : positions(positions), uvs(uvs) {
    if (positions.size() != uvs.size()) {
        throw std::runtime_error("UVRegion: positions and uvs must have same size");
    }
}

bool UVRegion::contains(const vec2& p, float eps) const {
    size_t n = positions.size();
    if (n < 3) return false;

    // Check if point is on boundary (within epsilon)
    for (size_t i = 0; i < n; ++i) {
        const vec2& a = positions[i];
        const vec2& b = positions[(i + 1) % n];
        
        vec2 ab = b - a;
        float abLen2 = glm::dot(ab, ab);
        float t = glm::clamp(glm::dot(p - a, ab) / (abLen2 + 1e-20f), 0.0f, 1.0f);
        vec2 proj = a + t * ab;
        
        if (glm::length(p - proj) <= eps) return true;
        if (glm::length(p - a) <= eps) return true;
        if (glm::length(p - b) <= eps) return true;
    }

    // Standard even-odd raycast test
    bool inside = false;
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const vec2& a = positions[i];
        const vec2& b = positions[j];
        
        bool intersect = ((a.y > p.y) != (b.y > p.y)) &&
                        (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y + 1e-12f) + a.x);
        if (intersect) inside = !inside;
    }
    
    return inside;
}

float UVRegion::distance(const vec2& pos) const {
    if (contains(pos, 0.0f)) return 0.0f;
    
    size_t n = positions.size();
    float minDist = std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < n; ++i) {
        const vec2& a = positions[i];
        const vec2& b = positions[(i + 1) % n];
        
        vec2 ab = b - a;
        float t = glm::clamp(glm::dot(pos - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
        vec2 closest = a + t * ab;
        
        float dist = glm::length(pos - closest);
        minDist = std::min(minDist, dist);
    }
    
    return minDist;
}

vec2 UVRegion::sampleUV(const vec2& pos) const {
    if (positions.size() < 3) return vec2(0, 0);
    
    // Find closest point on region
    vec2 closestPoint = pos;
    if (!contains(pos, 0.0f)) {
        float minDist = std::numeric_limits<float>::max();
        for (size_t i = 0; i < positions.size(); ++i) {
            const vec2& a = positions[i];
            const vec2& b = positions[(i + 1) % positions.size()];
            
            vec2 ab = b - a;
            float t = glm::clamp(glm::dot(pos - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
            vec2 candidate = a + t * ab;
            
            float dist = glm::length(pos - candidate);
            if (dist < minDist) {
                minDist = dist;
                closestPoint = candidate;
            }
        }
    }
    
    // Triangulate region to find which triangle contains the point
    std::vector<std::vector<std::array<double, 2>>> polygon;
    polygon.emplace_back();
    polygon[0].reserve(positions.size());
    for (const vec2& v : positions) {
        polygon[0].push_back({{static_cast<double>(v.x), static_cast<double>(v.y)}});
    }
    
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);
    
    // Find triangle containing closestPoint
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];
        
        const vec2& a = positions[i0];
        const vec2& b = positions[i1];
        const vec2& c = positions[i2];
        
        // Check if point is in this triangle using barycentric coordinates
        float denom = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
        if (std::abs(denom) < 1e-8f) continue;
        
        float w1 = ((b.y - c.y) * (closestPoint.x - c.x) + (c.x - b.x) * (closestPoint.y - c.y)) / denom;
        float w2 = ((c.y - a.y) * (closestPoint.x - c.x) + (a.x - c.x) * (closestPoint.y - c.y)) / denom;
        float w3 = 1.0f - w1 - w2;
        
        // Check if point is inside triangle (with small epsilon for boundary)
        const float eps = -1e-6f;
        if (w1 >= eps && w2 >= eps && w3 >= eps) {
            // Clamp to valid range
            w1 = glm::clamp(w1, 0.0f, 1.0f);
            w2 = glm::clamp(w2, 0.0f, 1.0f);
            w3 = glm::clamp(w3, 0.0f, 1.0f);
            
            // Interpolate UVs
            return uvs[i0] * w1 + uvs[i1] * w2 + uvs[i2] * w3;
        }
    }
    
    // Fallback: find nearest vertex
    float minDist = std::numeric_limits<float>::max();
    size_t nearestIdx = 0;
    for (size_t i = 0; i < positions.size(); ++i) {
        float dist = glm::length(closestPoint - positions[i]);
        if (dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }
    
    return uvs[nearestIdx];
}

void UVRegion::flipUVx() {
    if (uvs.empty()) return;
    
    float minU = uvs[0].x;
    float maxU = uvs[0].x;
    for (const vec2& uv : uvs) {
        minU = std::min(minU, uv.x);
        maxU = std::max(maxU, uv.x);
    }
    
    float range = maxU - minU;
    if (range < 1e-12f) return;
    
    for (vec2& uv : uvs) {
        uv.x = maxU - (uv.x - minU);
    }
}

void UVRegion::flipHorizontal() {
    for (vec2& p : positions) {
        p.x *= -1.0f;
    }
}