#ifndef EDGER_H
#define EDGER_H

#include "util/includes.h"
#include "util/maths.h"

struct Edger {
    std::vector<vec2> region;

    Edger(const std::vector<vec2> region);
    vec2 getNearestEdgeIntersection(const vec2& pos, const vec2& dir);
    vec2 getNearestEdgePoint(const vec2& pos);
    std::pair<int, int> getVertexRangeBelowThreshold(const vec2& dir, float thresh, const vec2& start);
    void addRangeInside(std::vector<vec2>& vecs, const std::pair<int, int> range);
    bool getEdgeIntersection(int edgeStartIndex, const vec2& pos, const vec2& dir, vec2& out);
    void reflectVerticesOverLine(std::vector<vec2>& reflected, int a, int b, const vec2& pos, const vec2& dir);
    void addRangeOutside(std::vector<vec2>& unreflected, int a, int b);

    // cut specific 
    void flipHorizontal();

    // tesing functions
    bool isPointOutside(const vec2& p, float eps=1e-8f) const;

    // cleaning functions
    void removeAll(const std::vector<vec2> removes, float epsilon=1e-6f);
    void keepOnly(const std::vector<vec2> keeps, float epsilon=1e-6f);
    void pruneDups();
};

inline void flipVecsHorizontal(std::vector<vec2>& vecs) {
    for (vec2& v : vecs) v.x *= -1;
}

#endif