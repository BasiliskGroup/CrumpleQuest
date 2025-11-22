#ifndef CLIPPER_HELPER_H
#define CLIPPER_HELPER_H

#include "util/includes.h"
#include "clipper2/clipper.h"

using namespace Clipper2Lib;

// Scale for converting float -> int64
constexpr double CLIPPER_SCALE = 1e10;

Paths64 makePaths64FromRegion(const std::vector<vec2>& region);
std::vector<vec2> makeRegionFromPaths64(const Paths64& paths);
std::vector<vec2> simplifyCollinear(const std::vector<vec2>& region, float epsilon=EPSILON);

#endif
