#include "levels/dymesh.h"

// helper functions TODO move to own file
std::vector<uint32_t> earcutIndicesFromRegion(const std::vector<vec2>& region) {
    // earcut expects vector< vector< Point > > where each point is an array of two numbers.
    std::vector<std::vector<std::array<double, 2>>> polygon;
    polygon.emplace_back();
    polygon[0].reserve(region.size());
    for (const vec2& v : region) polygon[0].push_back({ { static_cast<double>(v.x), static_cast<double>(v.y) } });

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

DyMesh::DyMesh(const std::vector<vec2>& region, const std::vector<Tri>& data) : Edger(region), data(data) {
    ensureCCW(this->region);
}

DyMesh::DyMesh(const std::vector<vec2>& region, Mesh* mesh) : Edger(region), data() {
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

DyMesh::DyMesh(const std::vector<vec2>& region) : Edger(region), data() {
    ensureCCW(this->region);

    // triangulate region with earcut
    std::vector<uint32_t> indices = earcutIndicesFromRegion(region);
    // data.reserve(indices.size() / 3);
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        Vert v0(region[indices[i + 0]], {0, 0});
        Vert v1(region[indices[i + 1]], {0, 1});
        Vert v2(region[indices[i + 2]], {1, 1});
        std::array<Vert, 3> verts = { v0, v1, v2 };
        data.emplace_back(verts);
    }
}

void DyMesh::cut(const std::vector<vec2>& clipRegion) {
    if (clipRegion.empty()) return;

    std::vector<Tri> newData;
    newData.reserve(data.size());

    for (const Tri& tri : data) {
        std::vector<vec2> triPoly = tri.toPolygon();  // 3 vertices
        Paths64 subj = makePaths64FromRegion(triPoly);
        Paths64 clip = makePaths64FromRegion(clipRegion);
        Paths64 sol;

        try {
            sol = Difference(subj, clip, FillRule::NonZero);
        } catch (...) {
            continue;
        }

        // If nothing remains, triangle fully cut away
        if (sol.empty()) continue;

        // Each path in solution = one clipped polygon piece
        for (auto& p : sol) {
            std::vector<vec2> clipped = makeRegionFromPaths64({p});
            if (clipped.size() < 3) continue;

            ensureCCW(clipped);
            auto indices = earcutIndicesFromRegion(clipped);

            for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                vec2 p0 = clipped[indices[i]];
                vec2 p1 = clipped[indices[i+1]];
                vec2 p2 = clipped[indices[i+2]];

                // Sample UVs from *original triangle* for more local accuracy
                Vert v0(p0, tri.sampleUV(p0));
                Vert v1(p1, tri.sampleUV(p1));
                Vert v2(p2, tri.sampleUV(p2));
                newData.emplace_back(std::array<Vert,3>{v0, v1, v2});
            }
        }
    }

    // Rebuild region if needed
    Paths64 subjAll = makePaths64FromRegion(region);
    Paths64 clip = makePaths64FromRegion(clipRegion);
    Paths64 sol = Difference(subjAll, clip, FillRule::NonZero);
    region = makeRegionFromPaths64(sol);

    data = std::move(newData);
}


// cut overload that accepts another DyMesh
void DyMesh::cut(const DyMesh& other) {
    cut(other.region);
}

bool DyMesh::copy(const DyMesh& other) {
    // For every triangle vertex in this mesh, sample UV from other
    for (Tri& t : data) {
        for (int i = 0; i < 3; ++i) {
            vec2 p = t.verts[i].pos;
            vec2 uv;
             if (other.sampleUV(p, uv) == false) return false;
            t.verts[i].uv = uv;
        }
    }
    return true;
}

void DyMesh::paste(const DyMesh& other) {
    if (other.region.empty()) return;

    // 3) Deep copy incoming triangles
    for (const Tri& t : other.data) {
        data.push_back(t);
    }

    // 4) Update region to union
    Paths64 a = makePaths64FromRegion(this->region);
    Paths64 b = makePaths64FromRegion(other.region);

    Paths64 unionSol;
    try {
        unionSol = Union(a, b, FillRule::NonZero);
    } catch (...) {
        std::cout << "  PASTE: Union failed!" << std::endl;
        return;
    }

    std::vector<vec2> newRegion = makeRegionFromPaths64(unionSol);
    
    if (!newRegion.empty()) {
        ensureCCW(newRegion);
        region = std::move(newRegion);
    }
}

DyMesh DyMesh::mirror(const vec2& pos, const vec2& dir) {
    // Normalize the direction
    vec2 nDir = glm::normalize(dir);

    // Helper to mirror a point across a line
    auto mirrorPoint = [&](const vec2& p) -> vec2 {
        vec2 rel = p - pos;
        float proj = glm::dot(rel, nDir);        // use float
        vec2 alongLine = proj * nDir;            // safe now
        vec2 perp = rel - alongLine;
        return pos + alongLine - perp;           // mirror perpendicular component
    };

    // Mirror the region
    std::vector<vec2> mirroredRegion(region.size());
    for (size_t i = 0; i < region.size(); ++i) {
        mirroredRegion[i] = mirrorPoint(region[i]);
    }

    // Ensure CCW winding
    ensureCCW(mirroredRegion);

    // Mirror the triangles
    std::vector<Tri> mirroredData(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        const Tri& t = data[i];
        std::array<Vert, 3> mirroredVerts;
        for (int j = 0; j < 3; ++j) {
            const Vert& v = t.verts[j];
            vec2 mirroredPos = mirrorPoint(v.pos);
            vec2 mirroredUV = v.uv; // Usually UVs are kept relative, mirror if needed
            mirroredVerts[j] = Vert(mirroredPos, mirroredUV);
        }
        // Reverse winding to maintain CCW
        std::swap(mirroredVerts[1], mirroredVerts[2]);
        mirroredData[i] = Tri(mirroredVerts);
    }

    // Construct and return new DyMesh
    DyMesh mirroredMesh(mirroredRegion, mirroredData);
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

void DyMesh::printData() {
    std::cout << "===== PRINT DATA ====" << std::endl;
    for (const Tri& tri : data) {
        tri.print();
    }
}