#include "levels/dymesh.h"

#include <stdexcept>
#include <limits>
#include <cmath>
#include <algorithm>

// -- Clipper2 namespace alias
using namespace Clipper2Lib;

// scale for converting float -> int64 to preserve precision
static constexpr double CLIPPER_SCALE = 1e6;

// ----------------------
// Helpers: polygon & clipping
// ----------------------
static Paths64 makePaths64FromRegion(const std::vector<vec2>& region) {
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

static std::vector<vec2> makeRegionFromPaths64(const Paths64& paths) {
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

// compute signed area of polygon (positive = CCW)
static float signedArea(const std::vector<vec2>& poly) {
    double a = 0.0;
    size_t n = poly.size();
    if (n < 3) return 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const vec2& p0 = poly[i];
        const vec2& p1 = poly[(i + 1) % n];
        a += (double)p0.x * (double)p1.y - (double)p1.x * (double)p0.y;
    }
    return static_cast<float>(0.5 * a);
}

static void ensureCCW(std::vector<vec2>& poly) {
    if (signedArea(poly) < 0.0f) std::reverse(poly.begin(), poly.end());
}

// ----------------------
// Helpers: earcut triangulation
// ----------------------
using Coord = double;
using PointForEar = std::array<Coord, 2>;

static std::vector<uint32_t> earcutIndicesFromRegion(const std::vector<vec2>& region) {
    // earcut expects vector< vector< Point > > where each point is an array of two numbers.
    std::vector<std::vector<PointForEar>> polygon;
    polygon.emplace_back();
    polygon[0].reserve(region.size());
    for (const vec2& v : region) polygon[0].push_back({ { static_cast<Coord>(v.x), static_cast<Coord>(v.y) } });

    return mapbox::earcut<uint32_t>(polygon);
}

// ----------------------
// Helpers: sampling UVs from a Tri list
// ----------------------
static vec2 sampleUVFromTriList(const std::vector<Tri>& tris, const vec2& pos) {
    // Try direct contain test first
    for (const Tri& t : tris) {
        if (t.contains(pos)) {
            return t.sampleUV(pos);
        }
    }

    // Fallback: sample from nearest triangle centroid
    if (tris.empty()) return vec2(0.0f, 0.0f);
    size_t best = 0;
    float bestD = std::numeric_limits<float>::infinity();
    for (size_t i = 0; i < tris.size(); ++i) {
        vec2 a = tris[i].verts[0].pos;
        vec2 b = tris[i].verts[1].pos;
        vec2 c = tris[i].verts[2].pos;
        vec2 centroid((a.x + b.x + c.x) / 3.0f, (a.y + b.y + c.y) / 3.0f);
        float dx = centroid.x - pos.x;
        float dy = centroid.y - pos.y;
        float d = dx*dx + dy*dy;
        if (d < bestD) { bestD = d; best = i; }
    }
    vec2 centroid = (tris[best].verts[0].pos + tris[best].verts[1].pos + tris[best].verts[2].pos) / 3.0f;
    return tris[best].sampleUV(centroid);
}

// ----------------------
// Constructors
// ----------------------
DyMesh::DyMesh(const std::vector<vec2>& region_, const std::vector<Tri>& data_) {
    region = region_;
    ensureCCW(region);
    data = data_;
}

DyMesh::DyMesh(Mesh* mesh) {
    if (mesh == nullptr) {
        throw std::invalid_argument("DyMesh(Mesh*): mesh pointer is null");
    }
    throw std::runtime_error(
        "DyMesh(Mesh*): conversion from Mesh to DyMesh not implemented here by design. "
        "Implement this constructor externally to match your Mesh layout."
    );
}

// ----------------------
// cut: removes the given polygon region from this->region, re-triangulates, resamples UVs
// ----------------------
void DyMesh::cut(const std::vector<vec2>& clipRegion) {
    if (clipRegion.empty()) return;

    // backup old data to resample from
    std::vector<Tri> oldData = data;

    // Clipper input
    Paths64 subj = makePaths64FromRegion(this->region);
    Paths64 clip = makePaths64FromRegion(clipRegion);

    // Perform difference: subj - clip
    Paths64 solution;
    try {
        solution = Difference(subj, clip, FillRule::NonZero);
    } catch (...) {
        // If Clipper fails for any reason, avoid corrupting the mesh
        return;
    }

    // Convert solution to a single region (pick the largest path)
    std::vector<vec2> newRegion = makeRegionFromPaths64(solution);

    if (newRegion.empty()) {
        // fully cut away - clear data & region
        region.clear();
        data.clear();
        return;
    }

    ensureCCW(newRegion);
    region = newRegion;

    // Re-triangulate region with earcut
    std::vector<uint32_t> indices = earcutIndicesFromRegion(region);

    // Rebuild data triangles and resample UVs from oldData
    std::vector<Tri> newData;
    newData.reserve(indices.size() / 3);
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        Vert v0(region[indices[i + 0]], sampleUVFromTriList(oldData, region[indices[i + 0]]));
        Vert v1(region[indices[i + 1]], sampleUVFromTriList(oldData, region[indices[i + 1]]));
        Vert v2(region[indices[i + 2]], sampleUVFromTriList(oldData, region[indices[i + 2]]));
        std::array<Vert, 3> verts = { v0, v1, v2 };
        newData.emplace_back(verts);
    }

    data = std::move(newData);
}

// cut overload that accepts another DyMesh
void DyMesh::cut(const DyMesh& other) {
    cut(other.region);
}

// ----------------------
// copy: override this mesh's UVs by sampling from the other DyMesh
// ----------------------
void DyMesh::copy(const DyMesh& other) {
    // For every triangle vertex in this mesh, sample UV from other
    for (Tri& t : data) {
        for (int i = 0; i < 3; ++i) {
            vec2 p = t.verts[i].pos;
            vec2 uv = other.sampleUV(p);
            t.verts[i].uv = uv;
        }
    }
}

// ----------------------
// paste: cut incoming from this, re-triangulate this part, then append incoming triangles
// and union regions to produce the new region. For pasted triangles we keep their UVs as-is.
// ----------------------
void DyMesh::paste(const DyMesh& other) {
    if (other.region.empty()) return;

    // 1) Make a backup of our current data to resample UVs after the cut
    std::vector<Tri> oldData = data;

    // 2) Cut the incoming region from ourselves (this updates this->region and this->data)
    cut(other.region);

    // 3) Deep copy incoming triangles into our data (their UVs preserved)
    //    Note: we simply append; the caller requested that pasted triangles come from the incoming DyMesh.
    for (const Tri& t : other.data) {
        data.push_back(t); // Tri's default copy is a deep copy for our Vert array
    }

    // 4) Update region to be union(this->region, other.region) using Clipper2
    Paths64 a = makePaths64FromRegion(this->region);
    Paths64 b = makePaths64FromRegion(other.region);

    // If our region is empty (we were fully cut), a will be empty; union should then just be b
    Paths64 unionSol;
    try {
        unionSol = Union(a, b, FillRule::NonZero);
    } catch (...) {
        // union failed; bail out but keep current data (which contains appended triangles)
        return;
    }

    std::vector<vec2> newRegion = makeRegionFromPaths64(unionSol);
    if (!newRegion.empty()) {
        ensureCCW(newRegion);
        region = std::move(newRegion);
    }
    // Done. We intentionally do NOT re-earcut the unioned region into a single triangle set,
    // because you requested to keep the incoming triangles as-is (so pasted triangles retain their UVs).
    // If you prefer a final single earcut triangulation of the unioned region, we can add that as an option.
}

// ----------------------
// sampleUV: find containing triangle in this->data and return sampled UV. Fallback to nearest triangle centroid.
// ----------------------
vec2 DyMesh::sampleUV(const vec2& pos) const {
    // check for containing triangle first
    for (const Tri& t : data) {
        if (t.contains(pos)) {
            return t.sampleUV(pos);
        }
    }
    
    throw std::runtime_error("invalid copy location");
}

void DyMesh::toData(std::vector<float>& data) {
    data.clear();
    data.reserve(this->data.size() * 3 * 5);

    for (uint i = 0; i < this->data.size(); i++) {
        for (const Vert& vert : this->data[i].verts) {
            data.push_back(vert.pos.x);
            data.push_back(vert.pos.y);
            data.push_back(0.0);
            data.push_back(vert.uv.x);
            data.push_back(vert.uv.y);
        }
    }
}