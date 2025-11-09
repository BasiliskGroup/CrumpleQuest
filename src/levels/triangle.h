#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "util/includes.h"
#include "util/maths.h"

struct Tri {
    Vec2Triplet verts;

    Tri(Vec2Triplet verts) : verts(verts) {}
    bool contains(const vec2& pos) const;
    vec2 intersect(const vec2& pos, const vec2& dir);
    vec2 leastDot(const vec2& dir);
};

#endif