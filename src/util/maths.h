#ifndef MATHS_H
#define MATHS_H

#include "util/includes.h"

void tripleProduct(vec2& a, vec2& b, vec2& c, vec2& o);
void perpTowards(vec2& v, vec2& to, vec2& perp);
void transform(vec2& pos, mat2x2& mat, vec2& v);

#endif