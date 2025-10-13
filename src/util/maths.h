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

#endif