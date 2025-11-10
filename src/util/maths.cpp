#include "util/maths.h"

void tripleProduct(const vec2& a, const vec2& b, const vec2& c, vec2& o) {
    o = glm::dot(a, c) * b - glm::dot(a, b) * c;
}

void perpTowards(const vec2& v, const vec2& to, vec2& perp) {
    // Two possible perpendiculars
    vec2 left  = vec2(-v.y,  v.x);
    vec2 right = vec2( v.y, -v.x);

    // Pick whichever points more toward 'to'
    perp = (glm::dot(left, to) > glm::dot(right, to)) ? left : right;
}


/**
 * @brief Transforms the vector v using the position vector and scale/rotation matrix
 * 
 * @param pos 
 * @param mat 
 * @param v 
 */
void transform(const vec2& pos, const mat2x2& mat, vec2& v) {
    v = mat * v + pos;
}

float cross(const vec2& a, const vec2& b) {
    return a.x * b.y - a.y * b.x;
}

float sign(const vec2& a, const vec2& b, const vec2& c) {
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

bool isCCW(const vec2& a, const vec2& b, const vec2& c) {
    return sign(a, b, c) > 0;
}

bool intersectLineSegmentInfiniteLine(
    const vec2& a0, const vec2& a1,
    const vec2& b0, const vec2& bDir,
    vec2& outIntersection)
{
    vec2 aDir = a1 - a0;
    float det = aDir.x * (-bDir.y) + aDir.y * bDir.x;
    
    // Parallel lines
    if (fabs(det) < 1e-8f)
        return false;
    
    vec2 diff = b0 - a0;
    float tA = (diff.x * (-bDir.y) + diff.y * bDir.x) / det;
    // float tB = (aDir.x * diff.y - aDir.y * diff.x) / det;  // Not needed for infinite line
    
    // Only valid if intersection lies on segment A
    // The infinite line extends in both directions, so no check on tB
    if (tA >= 0.0f && tA <= 1.0f) {
        outIntersection = a0 + tA * aDir;
        return true;
    }
    return false;
}

vec2 nearestPointOnEdgeToPoint(const vec2& start, const vec2& end, const vec2& point) {
    vec2 edge = end - start;
    vec2 toPoint = point - start;

    float edgeLenSq = glm::dot(edge, edge);
    if (edgeLenSq < 1e-8f)
        return toPoint; // Degenerate edge (both endpoints same)

    // Project point onto line, then clamp projection to segment range
    float t = glm::dot(toPoint, edge) / edgeLenSq;
    t = glm::clamp(t, 0.0f, 1.0f);

    vec2 closest = start + t * edge;
    return closest;
}

float distancePointToEdge(const vec2& start, const vec2& end, const vec2& point) {
    return glm::length(point - nearestPointOnEdgeToPoint(start, end, point));
}

vec2 reflectPointOverLine(const vec2& pos, const vec2& dir, const vec2& point) {
    vec2 nDir = glm::normalize(dir);
    vec2 rel = point - pos;
    vec2 proj = glm::dot(rel, nDir) * nDir;
    vec2 perp = rel - proj;
    vec2 reflected = rel - 2.0f * perp;
    return pos + reflected;
}

float signedArea(const std::vector<vec2>& poly) {
    double a = 0.0;
    size_t n = poly.size();
    if (n < 3) return 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const vec2& p0 = poly[i];
        const vec2& p1 = poly[(i + 1) % n];
        a += (double)p0.x * (double)p1.y - (double)p1.x * (double)p0.y;
    }
    return static_cast<float>(0.5 * a);
}

void ensureCCW(std::vector<vec2>& poly) {
    if (signedArea(poly) < 0.0f) std::reverse(poly.begin(), poly.end());
}