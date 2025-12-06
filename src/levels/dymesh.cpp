#include "levels/dymesh.h"

DyMesh::DyMesh(const std::vector<vec2>& region, const std::vector<UVRegion>& regions) 
    : Edger(region), regions(regions) {
    ensureCCW(this->region);
}

DyMesh::DyMesh(const std::vector<vec2>& region, Mesh* mesh) : Edger(region), regions() {
    ensureCCW(this->region);
    
    std::vector<float>& verts = mesh->getVertices();
    
    // Group vertices into triangles, then merge into regions
    // For now, create one UVRegion per triangle (could be optimized later)
    uint i = 0;
    while (i < verts.size()) {
        std::vector<vec2> positions;
        std::vector<vec2> uvs;
        
        for (uint j = 0; j < 3; j++) {
            vec2 pos = {verts[i + 0], verts[i + 1]};
            vec2 uv = {verts[i + 3], verts[i + 4]};
            positions.push_back(pos);
            uvs.push_back(uv);
            i += 5;
        }
        
        // Create basis from triangle vertices
        // v0 is origin, basis vectors are (v1-v0) and (v2-v0)
        vec2 originUV = uvs[0];
        vec2 dir1_pos = positions[1] - positions[0];
        vec2 dir1_uv = uvs[1] - uvs[0];
        vec2 dir2_pos = positions[2] - positions[0];
        vec2 dir2_uv = uvs[2] - uvs[0];
        
        std::array<Vert, 2> basis = {
            Vert(dir1_pos, dir1_uv),
            Vert(dir2_pos, dir2_uv)
        };
        
        regions.emplace_back(positions, basis, originUV);
    }
}

DyMesh::DyMesh(const std::vector<vec2>& region) : Edger(region), regions() {
    ensureCCW(this->region);
    
    if (region.size() < 3) return;
    
    // Calculate bounding box for UV mapping
    vec2 minPos = region[0];
    vec2 maxPos = region[0];
    for (const vec2& p : region) {
        minPos.x = std::min(minPos.x, p.x);
        minPos.y = std::min(minPos.y, p.y);
        maxPos.x = std::max(maxPos.x, p.x);
        maxPos.y = std::max(maxPos.y, p.y);
    }
    
    vec2 size = maxPos - minPos;
    float maxDim = std::max(size.x, size.y);
    if (maxDim < 1e-6f) maxDim = 1.0f;
    
    // Create basis: direction vectors map position deltas to UV deltas
    // basis[0]: x-direction, basis[1]: y-direction
    std::array<Vert, 2> basis = {
        Vert(vec2(maxDim, 0.0f), vec2(1.0f, 0.0f)),
        Vert(vec2(0.0f, maxDim), vec2(0.0f, 1.0f))
    };
    
    // originUV: UV at region[0] (the origin position)
    vec2 originUV = (region[0] - minPos) / maxDim;
    
    regions.emplace_back(region, basis, originUV);
}

bool DyMesh::cut(const std::vector<vec2>& clipRegion, bool useIntersection) {
    if (clipRegion.empty()) return false;

    // Take snapshot of current state for UV sampling
    DyMesh snapshot = *this;

    // Update outer region boundary
    Paths64 subjAll = makePaths64FromRegion(region);
    Paths64 clip = makePaths64FromRegion(clipRegion);
    Paths64 sol = useIntersection ? Intersect(subjAll, clip, FillRule::NonZero) 
                                  : Difference(subjAll, clip, FillRule::NonZero);

    // Note: sol.size() > 1 means the cut created multiple pieces.
    // makeRegionFromPaths64 will return the largest piece, which is typically
    // the main region we want to keep.
    std::vector<vec2> newRegion = makeRegionFromPaths64(sol);

    if (newRegion.size() < 3) {
        region.clear();
        regions.clear();
        return true;
    }

    ensureCCW(newRegion);

    // Process each UV region
    std::vector<UVRegion> newRegions;
    
    // Helper function to check if point is inside polygon
    auto pointInPolygon = [](const vec2& p, const std::vector<vec2>& polygon) -> bool {
        if (polygon.size() < 3) return false;
        bool inside = false;
        for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            const vec2& a = polygon[i];
            const vec2& b = polygon[j];
            bool intersect = ((a.y > p.y) != (b.y > p.y)) &&
                            (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y + 1e-12f) + a.x);
            if (intersect) inside = !inside;
        }
        return inside;
    };
    
    for (const UVRegion& uvReg : snapshot.regions) {
        // For obstacles, check if they intersect with the clip region
        // If they do, they need to be clipped (even if all vertices are inside the new region)
        // This allows obstacles to be split when folded over
        if (uvReg.isObstacle) {
            // Check if obstacle intersects with clip region
            Paths64 obstaclePath = makePaths64FromRegion(uvReg.positions);
            Paths64 clipPaths = makePaths64FromRegion(clipRegion);
            Paths64 intersectionTest;
            
            try {
                intersectionTest = Intersect(obstaclePath, clipPaths, FillRule::NonZero);
            } catch (...) {
                intersectionTest.clear();
            }
            
            bool intersectsClipRegion = false;
            if (!intersectionTest.empty()) {
                for (const Path64& p : intersectionTest) {
                    if (Area(p) > 0) {
                        intersectsClipRegion = true;
                        break;
                    }
                }
            }
            
            // Only preserve obstacle as-is if:
            // 1. All vertices are inside the new region AND
            // 2. It does NOT intersect with the clip region (not being split)
            if (!intersectsClipRegion) {
                bool obstacleInside = true;
                for (const vec2& pos : uvReg.positions) {
                    if (!pointInPolygon(pos, newRegion)) {
                        obstacleInside = false;
                        break;
                    }
                }
                if (obstacleInside) {
                    // Preserve obstacle as-is if it's inside the new boundary and not being split
                    newRegions.push_back(uvReg);
                    continue;
                }
            }
            // If obstacle intersects clip region or is partially outside, clip it
        }
        
        Paths64 subj = makePaths64FromRegion(uvReg.positions);
        Paths64 clipPaths = makePaths64FromRegion(clipRegion);
        Paths64 regionSol;

        try {
            regionSol = useIntersection ? Intersect(subj, clipPaths, FillRule::NonZero)
                                       : Difference(subj, clipPaths, FillRule::NonZero);
        } catch (...) {
            continue;
        }

        if (regionSol.empty()) continue;

        // Each path is a clipped piece
        for (const Path64& path : regionSol) {
            std::vector<vec2> clippedPositions;
            clippedPositions.reserve(path.size());
            
            for (const Point64& pt : path) {
                clippedPositions.emplace_back(
                    static_cast<float>(pt.x / CLIPPER_SCALE),
                    static_cast<float>(pt.y / CLIPPER_SCALE)
                );
            }
            
            clippedPositions = simplifyCollinear(clippedPositions);
            if (clippedPositions.size() < 3) continue;
            
            ensureCCW(clippedPositions);

            // Preserve basis from original region, compute new originUV for new origin position
            vec2 newOriginUV = uvReg.sampleUV(clippedPositions[0]);
            newRegions.emplace_back(clippedPositions, uvReg.basis, newOriginUV, uvReg.isObstacle);
        }
    }

    region = std::move(newRegion);
    regions = std::move(newRegions);
    
    region = simplifyCollinear(region);
    pruneDups();

    return true;
}

bool DyMesh::cut(const DyMesh& other, bool useIntersection) {
    return cut(other.region, useIntersection);
}

bool DyMesh::copy(const DyMesh& other) {
    if (other.region.empty()) return false;

    // Create snapshot
    DyMesh temp = other;
    
    // Reverse our region for intersection
    std::vector<vec2> reverse = this->region;
    std::reverse(reverse.begin(), reverse.end());

    // Find overlap
    if (!temp.cut(reverse, true)) {
        return false;
    }

    if (temp.region.empty() || temp.regions.empty()) {
        return false;
    }

    this->regions = temp.regions;
    this->region = temp.region;

    return true;
}

bool DyMesh::paste(const DyMesh& other, int expected) {
    if (other.region.empty()) return false;

    // Deep copy incoming regions
    for (const UVRegion& uvReg : other.regions) {
        regions.push_back(uvReg);
    }

    // Update outer boundary to union
    Paths64 a = makePaths64FromRegion(this->region);
    Paths64 b = makePaths64FromRegion(other.region);

    Paths64 unionSol;
    try {
        unionSol = Union(a, b, FillRule::NonZero);
    } catch (...) {
        return false;
    }

    std::vector<vec2> newRegion = makeRegionFromPaths64(unionSol);

    if (newRegion.size() < 3) {
        if (expected > 0) {
            return false;
        }
        region.clear();
        regions.clear();
        return true;
    }

    ensureCCW(newRegion);

    if (expected != -1 && newRegion.size() != expected) {
        return false;
    }

    region = std::move(newRegion);
    region = simplifyCollinear(region);
    pruneDups();

    return true;
}

DyMesh* DyMesh::mirror(const vec2& pos, const vec2& dir) {
    vec2 nDir = glm::normalize(dir);

    auto mirrorPoint = [&](const vec2& p) -> vec2 {
        vec2 rel = p - pos;
        float proj = glm::dot(rel, nDir);
        vec2 alongLine = proj * nDir;
        vec2 perp = rel - alongLine;
        return pos + alongLine - perp;
    };

    // Mirror outer region
    std::vector<vec2> mirroredRegion(region.size());
    for (size_t i = 0; i < region.size(); ++i) {
        mirroredRegion[i] = mirrorPoint(region[i]);
    }
    ensureCCW(mirroredRegion);

    // Mirror UV regions
    std::vector<UVRegion> mirroredRegions;
    mirroredRegions.reserve(regions.size());
    
    for (const UVRegion& uvReg : regions) {
        std::vector<vec2> mirroredPositions;
        mirroredPositions.reserve(uvReg.positions.size());
        
        for (const vec2& p : uvReg.positions) {
            mirroredPositions.push_back(mirrorPoint(p));
        }
        
        ensureCCW(mirroredPositions);
        
        // Mirror the basis direction vectors as well
        // For direction vectors, we need to mirror them as vectors (not points)
        auto mirrorDir = [&](const vec2& d) -> vec2 {
            float proj = glm::dot(d, nDir);
            vec2 alongLine = proj * nDir;
            vec2 perp = d - alongLine;
            return alongLine - perp;
        };
        
        std::array<Vert, 2> mirroredBasis = {
            Vert(mirrorDir(uvReg.basis[0].pos), uvReg.basis[0].uv),
            Vert(mirrorDir(uvReg.basis[1].pos), uvReg.basis[1].uv)
        };
        
        mirroredRegions.emplace_back(mirroredPositions, mirroredBasis, uvReg.originUV, uvReg.isObstacle);
    }

    return new DyMesh(mirroredRegion, mirroredRegions);
}

bool DyMesh::sampleUV(const vec2& pos, vec2& uv) const {
    const float eps = 1e-6f;
    float minDistance = std::numeric_limits<float>::max();
    vec2 closestUV;
    bool found = false;

    for (const UVRegion& uvReg : regions) {
        float dist = uvReg.distance(pos);
        if (dist < minDistance) {
            minDistance = dist;
            closestUV = uvReg.sampleUV(pos);
            found = true;
        }
    }

    if (found && minDistance <= eps) {
        uv = closestUV;
        return true;
    }

    return false;
}

void DyMesh::toData(std::vector<float>& exp) {
    exp.clear();
    
    // Estimate capacity (assume ~2 triangles per region on average)
    exp.reserve(regions.size() * 2 * 3 * 5);

    for (const UVRegion& uvReg : regions) {
        if (uvReg.positions.size() < 3) continue;

        // Triangulate region
        std::vector<std::vector<std::array<double, 2>>> polygon;
        polygon.emplace_back();
        polygon[0].reserve(uvReg.positions.size());
        
        for (const vec2& v : uvReg.positions) {
            polygon[0].push_back({{static_cast<double>(v.x), static_cast<double>(v.y)}});
        }

        std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

        // Generate triangles
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            for (int j = 0; j < 3; j++) {
                uint32_t idx = indices[i + j];
                const vec2& pos = uvReg.positions[idx];
                vec2 uv = uvReg.sampleUV(pos);

                exp.push_back(pos.x);
                exp.push_back(-pos.y);
                exp.push_back(0.0f);
                exp.push_back(uv.x);
                exp.push_back(-uv.y);
            }
        }
    }
}

bool DyMesh::contains(const vec2& pos) const {
    for (const UVRegion& uvReg : regions) {
        if (uvReg.contains(pos)) return true;
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

    if (sol.empty())
        return false;

    for (const Path64& p : sol) {
        if (Area(p) > 0)
            return true;
    }

    return false;
}

void DyMesh::printData() {
    std::cout << "===== PRINT DATA ====" << std::endl;
    std::cout << "Regions: " << regions.size() << std::endl;
    for (size_t i = 0; i < regions.size(); ++i) {
        std::cout << "Region " << i << ": " << regions[i].positions.size() << " vertices" << std::endl;
        for (size_t j = 0; j < regions[i].positions.size(); ++j) {
            const vec2& p = regions[i].positions[j];
            vec2 uv = regions[i].sampleUV(p);
            std::cout << "  v" << j << ": pos(" << p.x << ", " << p.y 
                     << ") uv(" << uv.x << ", " << uv.y << ")" << std::endl;
        }
    }
}

void DyMesh::removeDataOutside() {
    ensureCCW(region);
    // Could implement region filtering here if needed
}

void DyMesh::flipHorizontal() {
    Edger::flipHorizontal();
    for (UVRegion& uvReg : regions) {
        uvReg.flipHorizontal();
    }
}