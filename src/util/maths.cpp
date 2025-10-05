#include "util/maths.h"

void tripleProduct(vec2& a, vec2& b, vec2& c, vec2& o) {
    o = glm::dot(a, c) * b - glm::dot(a, b) * c;
}

void perpTowards(vec2& v, vec2& to, vec2& perp) {
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
void transform(vec2& pos, mat2x2& mat, vec2& v) {
    v = mat * v + pos;
}