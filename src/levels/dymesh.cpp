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

vec2 sampleUVFromTriList(const std::vector<Tri>& tris, const vec2& pos) {
    // Try direct contain test first
    for (const Tri& t : tris) {
        if (t.contains(pos)) {
            return t.sampleUV(pos);
        }
    }
    throw std::runtime_error("Invalid copy location");
}

DyMesh::DyMesh(const std::vector<vec2>& region, const std::vector<Tri>& data) : region(region), data(data) {
    ensureCCW(this->region);
}

DyMesh::DyMesh(const std::vector<vec2>& region, Mesh* mesh) : region(region), data() {
    std::vector<float>& verts = mesh->getVertices();
    data.reserve(verts.size() / 15);

    uint i = 0;
    while (i < verts.size()) {
        std::array<Vert, 3> tri;
        for (uint j = 0; j < 3; j++) {
            tri[j] = Vert({verts[i++], verts[i++]}, {verts[++i], verts[++i]});
        }
        data.push_back(tri);
    }
}

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
}

vec2 DyMesh::sampleUV(const vec2& pos) const {
    std::cout << "internal sample" << std::endl;
    return sampleUVFromTriList(data, pos);
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