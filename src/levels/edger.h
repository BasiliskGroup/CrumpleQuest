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
    void addVertexRange(std::vector<vec2>& vecs, const std::pair<int, int> range);
    bool getEdgeIntersection(int edgeStartIndex, const vec2& pos, const vec2& dir, vec2& out);
    void reflectVerticesOverLine(std::vector<vec2>& reflected, int a, int b, const vec2& pos, const vec2& dir);
    void getUnreflectedVertices(std::vector<vec2>& unreflected, int a, int b);
};

#endif