#include "util/clipper_helper.h"

Paths64 makePaths64FromRegion(const std::vector<vec2>& region) {
    Paths64 paths;
    if (region.empty()) return paths;

    Path64 p;
    p.reserve(region.size());
    for (const vec2& v : region) {
        long long x = llround(v.x * CLIPPER_SCALE);
        long long y = llround(v.y * CLIPPER_SCALE);
        p.emplace_back(x, y);
    }
    paths.push_back(std::move(p));
    return paths;
}

std::vector<vec2> makeRegionFromPaths64(const Paths64& paths) {
    // Choose the largest (by absolute area) path as the single region
    if (paths.empty()) return {};

    auto areaPath64 = [](const Path64& p) {
        // Signed area using int64; return absolute value (in integer coords)
        // area64 = sum(x_i*y_{i+1} - x_{i+1}*y_i) / 2
        long double a = 0.0L;
        for (size_t i = 0, n = p.size(); i < n; ++i) {
            size_t j = (i + 1) % n;
            a += (long double)p[i].x * (long double)p[j].y - (long double)p[j].x * (long double)p[i].y;
        }
        return fabsl(a) * 0.5L;
    };

    size_t bestIdx = 0;
    long double bestA = -1.0L;
    for (size_t i = 0; i < paths.size(); ++i) {
        long double a = areaPath64(paths[i]);
        if (a > bestA) {
            bestA = a;
            bestIdx = i;
        }
    }

    const Path64& chosen = paths[bestIdx];
    std::vector<vec2> out;
    out.reserve(chosen.size());
    for (const Point64& pt : chosen) {
        out.emplace_back(static_cast<float>(pt.x / CLIPPER_SCALE), static_cast<float>(pt.y / CLIPPER_SCALE));
    }
    return out;
}