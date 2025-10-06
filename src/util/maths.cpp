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