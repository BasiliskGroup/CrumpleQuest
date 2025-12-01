#ifndef MATHS_H
#define MATHS_H

#include "util/includes.h"

void tripleProduct(const vec2& a, const vec2& b, const vec2& c, vec2& o);
void perpTowards(const vec2& v, const vec2& to, vec2& perp);
void transform(const vec2& pos, const mat2x2& mat, vec2& v);
float cross(const vec2& a, const vec2& b);

inline vec2 xy(const vec3& v) noexcept {
    return vec2(v.x, v.y);
}

float sign(const vec2& a, const vec2& b, const vec2& c);
bool isCCW(const vec2& a, const vec2& b, const vec2& c);

// folding helpers
bool intersectLineSegmentInfiniteLine(const vec2& a0, const vec2& a1,const vec2& b0, const vec2& bDir,vec2& outIntersection);
bool lineSegmentsIntersect(const vec2& a0, const vec2& a1, const vec2& b0, const vec2& b1);
bool lineSegmentIntersectsPolygon(const vec2& segStart, const vec2& segEnd, const std::vector<vec2>& polygon);
vec2 nearestPointOnEdgeToPoint(const vec2& start, const vec2& end, const vec2& point);
float distancePointToEdge(const vec2& start, const vec2& end, const vec2& point);
vec2 reflectPointOverLine(const vec2& pos, const vec2& dir, const vec2& point);

float signedArea(const std::vector<vec2>& poly);
void ensureCCW(std::vector<vec2>& poly);
void flipPolyY(std::vector<vec2>& poly);
std::pair<glm::vec3, glm::vec2> connectSquare(const glm::vec2& a, const glm::vec2& b);

#endif
