#ifndef EDGER_H
#define EDGER_H

#include "util/includes.h"
#include "util/maths.h"

struct Edger {
    std::vector<vec2> verts;

    Edger(const std::vector<vec2> verts);
    vec2 getNearestEdgeIntersection(const vec2& pos, const vec2& dir);
    vec2 getNearestEdgePoint(const vec2& pos);
};

#endif