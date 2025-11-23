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
    const float eps = 1e-6f;
    float minDistance = std::numeric_limits<float>::max();
    vec2 closestUV;
    bool found = false;

    for (const Tri& t : tris) {
        float dist = t.distance(pos);
        if (dist < minDistance) {
            minDistance = dist;
            closestUV = t.sampleUV(pos);
            found = true;
        }
    }

    if (found && minDistance <= eps) {
        uv = closestUV;
        return true;
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

bool DyMesh::cut(const std::vector<vec2>& clipRegion, bool useIntersection) {
    if (clipRegion.empty()) return false;

    std::vector<Tri> newData;
    newData.reserve(data.size());

    // TODO check if indivdual triangles were cut correctly
    // count remaining vertices? 
    for (const Tri& tri : data) {
        std::vector<vec2> triPoly = tri.toPolygon();
        Paths64 subj = makePaths64FromRegion(triPoly);
        Paths64 clip = makePaths64FromRegion(clipRegion);
        Paths64 sol;

        try {
            sol = useIntersection ? Intersect(subj, clip, FillRule::NonZero) : Difference(subj, clip, FillRule::NonZero);
        } catch (...) {
            continue;
        }

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

                Vert v0(p0, tri.sampleUV(p0));
                Vert v1(p1, tri.sampleUV(p1));
                Vert v2(p2, tri.sampleUV(p2));
                newData.emplace_back(std::array<Vert,3>{v0, v1, v2});
            }
        }
    }

    // Rebuild region - makeRegionFromPaths64 already extracts outer boundary
    Paths64 subjAll = makePaths64FromRegion(region);
    Paths64 clip = makePaths64FromRegion(clipRegion);
    Paths64 sol = useIntersection ? Intersect(subjAll, clip, FillRule::NonZero) : Difference(subjAll, clip, FillRule::NonZero);

    if (sol.size() > 1) {
        std::cout << "Polygon was cut into multiple pieces" << std::endl;
        return false;
    }
    
    std::vector<vec2> newRegion = makeRegionFromPaths64(sol);

    // we have completely deleted the region
    if (newRegion.size() < 3) {
        region.clear();
        data.clear();
        return true;
    }

    ensureCCW(newRegion);

    // all tests passed
    region = std::move(newRegion);
    data = std::move(newData);

    // remove unwanted vertices
    region = simplifyCollinear(region);
    pruneDups();
    removeDataOutside();

    return true;
}

// cut overload that accepts another DyMesh
bool DyMesh::cut(const DyMesh& other, bool useIntersection) {
    return cut(other.region, useIntersection);
}

bool DyMesh::copy(const DyMesh& other) {
    if (other.region.empty()) return false;

    DyMesh temp = DyMesh(other);
    std::vector reverse = this->region;
    std::reverse(reverse.begin(), reverse.end());

    // intersect cut to find overlap
    if (!temp.cut(reverse, true)) {
        return false;
    }

    if (temp.region.empty() || temp.data.empty()) {
        return false;
    }

    this->data = temp.data;
    this->region = temp.region;

    return true;
}


bool DyMesh::paste(const DyMesh& other, int expected) {
    if (other.region.empty()) return false;

    // Deep copy incoming triangles
    // no float inprecision here
    for (const Tri& t : other.data) {
        data.push_back(t);
    }

    // Update region to union - makeRegionFromPaths64 already extracts outer boundary
    Paths64 a = makePaths64FromRegion(this->region);
    Paths64 b = makePaths64FromRegion(other.region);

    Paths64 unionSol;
    try {
        unionSol = Union(a, b, FillRule::NonZero);
    } catch (...) {
        std::cout << "paste union failed" << std::endl;
        return false;
    }

    std::vector<vec2> newRegion = makeRegionFromPaths64(unionSol);

    if (newRegion.size() < 3) {
        if (expected > 0) {
            std::cout << "paste region is 0" << std::endl;
            return false;
        } 

        region.clear();
        data.clear();
        return true;
    }

    ensureCCW(newRegion);

    if (expected != -1 && newRegion.size() != expected) {
        std::cout << "paste incorrect vertex count " << newRegion.size() << " " << expected << std::endl;
        return false;
    }

    region = std::move(newRegion);
    region = simplifyCollinear(region);
    pruneDups();

    return true;
}

DyMesh* DyMesh::mirror(const vec2& pos, const vec2& dir) {
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
        if (data[i].distance(pos) < 1e-6f) {
            trindex = i;
            break;
        }
    }
    return trindex;
}   

bool DyMesh::contains(const vec2& pos) const {
    for (const Tri& tri : data) {
        if (tri.distance(pos) < 1e-6f) return true;
    }
    return false;
}

bool DyMesh::hasOverlap(const DyMesh& other) const {
    if (region.size() < 3 || other.region.size() < 3)
        return false;

    Paths64 subj = makePaths64FromRegion(region);
    Paths64 clip = makePaths64FromRegion(other.region);
    Paths64 sol;

    try {
        sol = Intersect(subj, clip, FillRule::NonZero);
    } catch (...) {
        return false;
    }

    // No intersection polygons at all
    if (sol.empty())
        return false;

    // Ensure overlap has a positive area (ignore touch-only intersections)
    for (const Path64& p : sol) {
        if (Area(p) > 0)      // Clipper area of path
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

void DyMesh::removeDataOutside() {
    // NOTE disable
    return;

    if (data.empty()) return;

    int i = static_cast<int>(data.size()) - 1;

    while (i >= 0) {

        bool remove = false;
        for (const Vert& v : data[i].verts) {
            if (isPointOutside(v.pos)) {
                remove = true;
                break;
            }
        }

        if (remove) {
            data.erase(data.begin() + i);
            std::cout << "cleaning" << std::endl;
        }

        i--;
    }
}

void DyMesh::flipHorizontal() {
    Edger::flipHorizontal();
    for (Tri& tri : data) {
        for (Vert& v : tri.verts) {
            v.pos.x *= -1;
        }
        // tri.flipUVx();
    }
}