#include "util/maths.h"

void tripleProduct(const vec2& a, const vec2& b, const vec2& c, vec2& o) {
    o = glm::dot(a, c) * b - glm::dot(a, b) * c;
}

void perpTowards(const vec2& v, const vec2& to, vec2& perp) {
    perp = vec2(-v[1], v[0]);
    if (glm::dot(perp, to) < 0) {
        perp = -perp;
    }
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