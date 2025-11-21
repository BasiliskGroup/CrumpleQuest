#include "levels/dymesh.h"

// Helper functions
std::vector<uint32_t> earcutIndicesFromRegion(const std::vector<vec2>& region) {
    std::vector<std::vector<std::array<double, 2>>> polygon;
    polygon.emplace_back();
    polygon[0].reserve(region.size());
    for (const vec2& v : region) {
        polygon[0].push_back({ { static_cast<double>(v.x), static_cast<double>(v.y) } });
    }
    return mapbox::earcut<uint32_t>(polygon);
}

bool sampleUVFromTriList(const std::vector<Tri>& tris, const vec2& pos, vec2& uv) {
    for (const Tri& t : tris) {
        if (t.contains(pos)) {
            uv = t.sampleUV(pos);
            return true;
        }
    }
    return false;
}

DyMesh::DyMesh(const std::vector<Point64>& region, const std::vector<Tri>& data) 
    : Edger(region), data(data) {
    ensureCCW(this->region);
}

DyMesh::DyMesh(const std::vector<Point64>& region, Mesh* mesh) 
    : Edger(region), data() {
    ensureCCW(this->region);
    
    std::vector<float>& verts = mesh->getVertices();
    data.reserve(verts.size() / 15);

    uint i = 0;
    while (i < verts.size()) {
        std::array<Vert, 3> tri;
        for (uint j = 0; j < 3; j++) {
            tri[j] = Vert({verts[i + 0], verts[i + 1]}, {verts[i + 3], verts[i + 4]});
            i += 5;
        }
        data.push_back(tri);
    }
}

DyMesh::DyMesh(const std::vector<Point64>& region) 
    : Edger(region), data() {
    ensureCCW(this->region);

    // Convert to vec2 for earcut (it needs doubles anyway)
    std::vector<vec2> vec2region;
    toFloat(vec2region);

    // Triangulate region with earcut
    std::vector<uint32_t> indices = earcutIndicesFromRegion(vec2region);
    
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        Vert v0(vec2region[indices[i + 0]], {0, 0});
        Vert v1(vec2region[indices[i + 1]], {0, 1});
        Vert v2(vec2region[indices[i + 2]], {1, 1});
        std::array<Vert, 3> verts = { v0, v1, v2 };
        data.emplace_back(verts);
    }
}

void DyMesh::cut(const std::vector<Point64>& clipRegion) {
    if (clipRegion.empty()) return;

    std::vector<Tri> newData;
    newData.reserve(data.size());

    // Process each triangle
    for (const Tri& tri : data) {
        // Convert triangle to Point64 polygon for Clipper
        std::vector<vec2> triVec2 = tri.toPolygon();
        Path64 triPath64;
        triPath64.reserve(3);
        for (const vec2& v : triVec2) {
            triPath64.push_back(vec2ToPoint64(v));
        }

        Paths64 subj = { triPath64 };
        Paths64 clip = { clipRegion };
        Paths64 sol;

        try {
            sol = Difference(subj, clip, FillRule::NonZero);
        } catch (...) {
            continue;
        }

        if (sol.empty()) continue;

        // Each path in solution = one clipped polygon piece
        for (const Path64& clippedPath : sol) {
            if (clippedPath.size() < 3) continue;

            // Convert back to vec2 for triangulation
            std::vector<vec2> clippedVec2;
            clippedVec2.reserve(clippedPath.size());
            for (const Point64& p : clippedPath) {
                clippedVec2.push_back(point64ToVec2(p));
            }

            ensureCCW(clippedVec2);
            auto indices = earcutIndicesFromRegion(clippedVec2);

            for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                vec2 p0 = clippedVec2[indices[i]];
                vec2 p1 = clippedVec2[indices[i+1]];
                vec2 p2 = clippedVec2[indices[i+2]];

                Vert v0(p0, tri.sampleUV(p0));
                Vert v1(p1, tri.sampleUV(p1));
                Vert v2(p2, tri.sampleUV(p2));
                newData.emplace_back(std::array<Vert,3>{v0, v1, v2});
            }
        }
    }

    // Rebuild region using Clipper (stays in Point64)
    Paths64 subjAll = { region };
    Paths64 clip = { clipRegion };
    Paths64 sol = Difference(subjAll, clip, FillRule::NonZero);
    
    if (!sol.empty()) {
        // Find largest path by area
        size_t bestIdx = 0;
        double bestArea = 0.0;
        for (size_t i = 0; i < sol.size(); ++i) {
            double area = std::abs(Area(sol[i]));
            if (area > bestArea) {
                bestArea = area;
                bestIdx = i;
            }
        }
        
        region = sol[bestIdx];
        ensureCCW(region);
        // TODO: Consider adding simplifyCollinear for Point64
    } else {
        region.clear(); // Completely cut away
    }

    data = std::move(newData);
}

bool DyMesh::copyIntersection(const DyMesh& other) {
    if (region.empty() || other.region.empty())
        return false;

    Paths64 subj = { region };
    Paths64 clip = { other.region };
    Paths64 sol;

    try {
        sol = Intersect(subj, clip, FillRule::NonZero);
    } catch (...) {
        std::cout << "  COPY INTERSECTION: Intersect failed!" << std::endl;
        return false;
    }

    if (sol.empty())
        return false;

    // Find largest intersection path
    size_t bestIdx = 0;
    double bestArea = 0.0;
    for (size_t i = 0; i < sol.size(); ++i) {
        double area = std::abs(Area(sol[i]));
        if (area > bestArea) {
            bestArea = area;
            bestIdx = i;
        }
    }
    
    Path64 newRegion = sol[bestIdx];
    if (newRegion.size() < 3)
        return false;

    ensureCCW(newRegion);

    // Convert to vec2 for triangulation
    std::vector<vec2> newRegionVec2;
    newRegionVec2.reserve(newRegion.size());
    for (const Point64& p : newRegion) {
        newRegionVec2.push_back(point64ToVec2(p));
    }

    // Triangulate intersection
    std::vector<uint32_t> indices = earcutIndicesFromRegion(newRegionVec2);
    if (indices.empty())
        return false;

    std::vector<Tri> newData;
    newData.reserve(indices.size() / 3);

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        vec2 p0 = newRegionVec2[indices[i + 0]];
        vec2 p1 = newRegionVec2[indices[i + 1]];
        vec2 p2 = newRegionVec2[indices[i + 2]];

        vec2 uv0, uv1, uv2;
        bool ok0 = other.sampleUV(p0, uv0);
        bool ok1 = other.sampleUV(p1, uv1);
        bool ok2 = other.sampleUV(p2, uv2);

        if (!(ok0 && ok1 && ok2)) {
            continue;
        }

        std::array<Vert, 3> verts = {
            Vert(p0, uv0),
            Vert(p1, uv1),
            Vert(p2, uv2)
        };

        newData.emplace_back(verts);
    }

    if (newData.empty())
        return false;

    region = std::move(newRegion);
    data = std::move(newData);

    return true;
}

void DyMesh::cut(const DyMesh& other) {
    cut(other.region);
}

bool DyMesh::copy(const DyMesh& other) {
    // For every triangle vertex in this mesh, sample UV from other
    for (Tri& t : data) {
        for (int i = 0; i < 3; ++i) {
            vec2 p = t.verts[i].pos;
            vec2 uv;
            if (!other.sampleUV(p, uv)) 
                return false;
            t.verts[i].uv = uv;
        }
    }
    return true;
}

void DyMesh::paste(const DyMesh& other) {
    if (other.region.empty()) return;

    // Deep copy incoming triangles
    for (const Tri& t : other.data) {
        data.push_back(t);
    }

    // Update region to union (stays in Point64)
    Paths64 a = { this->region };
    Paths64 b = { other.region };

    Paths64 unionSol;
    try {
        unionSol = Union(a, b, FillRule::NonZero);
    } catch (...) {
        std::cout << "  PASTE: Union failed!" << std::endl;
        return;
    }

    if (!unionSol.empty()) {
        // Find largest union path
        size_t bestIdx = 0;
        double bestArea = 0.0;
        for (size_t i = 0; i < unionSol.size(); ++i) {
            double area = std::abs(Area(unionSol[i]));
            if (area > bestArea) {
                bestArea = area;
                bestIdx = i;
            }
        }
        
        region = unionSol[bestIdx];
        ensureCCW(region);
    }
}

DyMesh* DyMesh::mirror(const Point64& p0, const Point64& p1) {
    // Helper to mirror a Point64 across a line defined by two points
    auto mirrorPoint64 = [&](const Point64& point) -> Point64 {
        Point64 lineDir = p1 - p0;
        Point64 pointToP0 = point - p0;
        
        int64_t lineDirDotLinDir = dot64(lineDir, lineDir);
        if (lineDirDotLinDir == 0) {
            return point; // Degenerate line
        }
        
        int64_t projection = dot64(pointToP0, lineDir);
        double t = static_cast<double>(projection) / static_cast<double>(lineDirDotLinDir);
        
        Point64 closestOnLine;
        closestOnLine.x = p0.x + static_cast<int64_t>(t * lineDir.x);
        closestOnLine.y = p0.y + static_cast<int64_t>(t * lineDir.y);
        
        // Reflect: 2 * closestOnLine - point
        Point64 reflected;
        reflected.x = 2 * closestOnLine.x - point.x;
        reflected.y = 2 * closestOnLine.y - point.y;
        
        return reflected;
    };

    // Helper to mirror vec2 for triangle vertices
    auto mirrorVec2 = [&](const vec2& v) -> vec2 {
        Point64 p64 = vec2ToPoint64(v);
        Point64 mirrored = mirrorPoint64(p64);
        return point64ToVec2(mirrored);
    };

    // Mirror the region (stays in Point64)
    std::vector<Point64> mirroredRegion(region.size());
    for (size_t i = 0; i < region.size(); ++i) {
        mirroredRegion[i] = mirrorPoint64(region[i]);
    }

    ensureCCW(mirroredRegion);

    // Mirror the triangles
    std::vector<Tri> mirroredData(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        const Tri& t = data[i];
        std::array<Vert, 3> mirroredVerts;
        for (int j = 0; j < 3; ++j) {
            const Vert& v = t.verts[j];
            vec2 mirroredPos = mirrorVec2(v.pos);
            vec2 mirroredUV = v.uv; // UVs stay the same
            mirroredVerts[j] = Vert(mirroredPos, mirroredUV);
        }
        // Reverse winding to maintain CCW after mirroring
        std::swap(mirroredVerts[1], mirroredVerts[2]);
        mirroredData[i] = Tri(mirroredVerts);
    }

    // Construct and return new DyMesh
    DyMesh* mirroredMesh = new DyMesh(mirroredRegion, mirroredData);
    return mirroredMesh;
}

bool DyMesh::sampleUV(const vec2& pos, vec2& uv) const {
    return sampleUVFromTriList(data, pos, uv);
}

void DyMesh::toData(std::vector<float>& exp) {
    exp.clear();
    exp.reserve(this->data.size() * 3 * 5);

    for (uint i = 0; i < this->data.size(); i++) {
        for (const Vert& vert : this->data[i].verts) {
            exp.push_back(vert.pos.x);
            exp.push_back(-vert.pos.y);
            exp.push_back(0.0);
            exp.push_back(vert.uv.x);
            exp.push_back(-vert.uv.y);
        }
    }
}

int DyMesh::getTrindex(const vec2& pos) const {
    int trindex = -1;
    for (int i = 0; i < data.size(); i++) {
        if (data[i].contains(pos)) {
            trindex = i;
            break;
        }
    }
    return trindex;
}   

bool DyMesh::contains(const vec2& pos) const {
    for (const Tri& tri : data) {
        if (tri.contains(pos)) return true;
    }
    return false;
}

bool DyMesh::hasOverlap(const DyMesh& other) const {
    if (region.size() < 3 || other.region.size() < 3)
        return false;

    Paths64 subj = { region };
    Paths64 clip = { other.region };
    Paths64 sol;

    try {
        sol = Intersect(subj, clip, FillRule::NonZero);
    } catch (...) {
        return false;
    }

    if (sol.empty())
        return false;

    // Ensure overlap has a positive area (ignore touch-only intersections)
    for (const Path64& p : sol) {
        if (Area(p) > 0)
            return true;
    }

    return false;
}

void DyMesh::printData() {
    std::cout << "===== PRINT DATA ====" << std::endl;
    for (const Tri& tri : data) {
        tri.print();
    }
}