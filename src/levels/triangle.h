#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "util/includes.h"
#include "util/maths.h"

struct Tri {
    Vec2Triplet verts;

    Tri(Vec2Triplet verts) : verts(verts) {}
    bool contains(const vec2& pos) const;
};

#endif