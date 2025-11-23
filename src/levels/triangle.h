#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "util/includes.h"
#include "util/maths.h"

struct Tri {
    std::array<Vert, 3> verts;

    Tri() : verts() {}
    Tri(std::array<Vert, 3> verts) : verts(verts) {}
    Tri(std::array<vec2, 3> verts);

    // Core methods
    float distance(const vec2& pos) const;
    float distance(const Vert& v) const { return distance(v.pos); }

    vec2 closestPointOnTriangle(const vec2& pos) const;

    vec2 intersect(const vec2& pos, const vec2& dir);
    vec2 intersect(const Vert& v, const vec2& dir) { return intersect(v.pos, dir); }

    vec2 leastDot(const vec2& dir);
    vec2 leastDot(const Vert& v) { return leastDot(v.pos); }

    vec2 sampleUV(const vec2& pos) const;
    vec2 sampleUV(const Vert& v) const { return sampleUV(v.pos); }

    void print() const;

    inline std::vector<vec2> toPolygon() const { return { verts[0].pos, verts[1].pos, verts[2].pos }; }

    void flipUVx();
};

#endif