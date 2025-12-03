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
    
    for (const UVRegion& uvReg : snapshot.regions) {
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

bool DyMesh::hasSharedEdge(const UVRegion& r1, const UVRegion& r2, std::vector<vec2>& sharedEdge) const {
    const float eps = 1e-5f;
    sharedEdge.clear();

    // Check each edge of r1 against each edge of r2
    for (size_t i = 0; i < r1.positions.size(); ++i) {
        const vec2& a1 = r1.positions[i];
        const vec2& b1 = r1.positions[(i + 1) % r1.positions.size()];

        for (size_t j = 0; j < r2.positions.size(); ++j) {
            const vec2& a2 = r2.positions[j];
            const vec2& b2 = r2.positions[(j + 1) % r2.positions.size()];

            // Check if edges overlap (same edge but opposite direction)
            bool match1 = (glm::length(a1 - b2) < eps && glm::length(b1 - a2) < eps);
            bool match2 = (glm::length(a1 - a2) < eps && glm::length(b1 - b2) < eps);

            if (match1 || match2) {
                sharedEdge.push_back(a1);
                sharedEdge.push_back(b1);
                return true;
            }
        }
    }

    return false;
}

bool DyMesh::hasCompatibleUVs(const UVRegion& r1, const UVRegion& r2, const std::vector<vec2>& sharedEdge) const {
    const float eps = 1e-5f;

    if (sharedEdge.size() != 2) return false;

    // Sample UVs at shared edge positions from both regions
    vec2 uv1_at_0 = r1.sampleUV(sharedEdge[0]);
    vec2 uv1_at_1 = r1.sampleUV(sharedEdge[1]);
    vec2 uv2_at_0 = r2.sampleUV(sharedEdge[0]);
    vec2 uv2_at_1 = r2.sampleUV(sharedEdge[1]);

    // Check if UVs match at shared vertices
    bool uv1Match = glm::length(uv1_at_0 - uv2_at_0) < eps;
    bool uv2Match = glm::length(uv1_at_1 - uv2_at_1) < eps;

    return uv1Match && uv2Match;
}

bool DyMesh::canMergeRegions(const UVRegion& r1, const UVRegion& r2, std::vector<vec2>& sharedEdge) const {
    if (r1.isObstacle || r2.isObstacle) return false;
    if (!hasSharedEdge(r1, r2, sharedEdge)) return false;
    if (!hasCompatibleUVs(r1, r2, sharedEdge)) return false;
    return true;
}

void DyMesh::cleanupDegenerateRegions() {
    const float eps = 1e-5f;
    
    for (size_t i = 0; i < regions.size(); ) {
        
        // Remove duplicate vertices
        std::vector<vec2> cleanedPos;
        
        for (size_t j = 0; j < regions[i].positions.size(); ++j) {
            bool isDup = false;
            for (size_t k = 0; k < cleanedPos.size(); ++k) {
                if (glm::length(regions[i].positions[j] - cleanedPos[k]) < eps) {
                    isDup = true;
                    break;
                }
            }
            if (!isDup) {
                cleanedPos.push_back(regions[i].positions[j]);
            }
        }
        
        // Check if region is degenerate after cleanup
        if (cleanedPos.size() < 3) {
            regions.erase(regions.begin() + i);
            continue;
        }
        
        // Calculate area
        float area = 0.0f;
        for (size_t j = 0; j < cleanedPos.size(); ++j) {
            const vec2& p1 = cleanedPos[j];
            const vec2& p2 = cleanedPos[(j + 1) % cleanedPos.size()];
            area += (p1.x * p2.y - p2.x * p1.y);
        }
        area = std::abs(area) * 0.5f;
        
        // Only remove truly tiny artifacts (< 0.01 square units)
        // This preserves small but legitimate triangles from fold operations
        const float minArea = 0.01f;
        if (area < minArea) {
            regions.erase(regions.begin() + i);
            continue;
        }
        
        // Update region if we removed duplicates (basis stays the same)
        if (cleanedPos.size() != regions[i].positions.size()) {
            regions[i].positions = cleanedPos;
        }
        
        ++i;
    }
}

UVRegion DyMesh::mergeTwo(const UVRegion& r1, const UVRegion& r2) const {
    // Use Clipper2 to union the two regions
    Paths64 paths1 = makePaths64FromRegion(r1.positions);
    Paths64 paths2 = makePaths64FromRegion(r2.positions);
    
    Paths64 unionResult;
    try {
        unionResult = Union(paths1, paths2, FillRule::NonZero);
    } catch (...) {
        // If union fails, return r1 unchanged
        return r1;
    }

    if (unionResult.empty() || unionResult.size() > 1) {
        // Union resulted in no region or multiple regions - shouldn't happen for adjacent regions
        return r1;
    }

    // Convert back to vec2
    std::vector<vec2> mergedPositions = makeRegionFromPaths64(unionResult);
    ensureCCW(mergedPositions);

    // Use r1's basis, compute originUV for new origin position
    // Preserve obstacle status - if either region is an obstacle, merged region is an obstacle
    bool mergedIsObstacle = r1.isObstacle || r2.isObstacle;
    vec2 newOriginUV = r1.sampleUV(mergedPositions[0]);
    return UVRegion(mergedPositions, r1.basis, newOriginUV, mergedIsObstacle);
}

void DyMesh::mergeAllRegions() {
    if (regions.size() < 2) {
        return;
    }
    
    // First cleanup pass
    cleanupDegenerateRegions();
    
    if (regions.size() < 2) {
        return;
    }
    
    bool anyMerged = true;
    
    while (anyMerged && regions.size() > 1) {
        anyMerged = false;
        
        // Try to merge each pair of regions
        for (size_t i = 0; i < regions.size(); ++i) {
            for (size_t j = i + 1; j < regions.size(); ++j) {
                std::vector<vec2> sharedEdge;
                
                // Check if regions share an edge
                bool hasShared = hasSharedEdge(regions[i], regions[j], sharedEdge);
                
                if (hasShared) {
                    // Check if UVs are compatible on the shared edge
                    if (!hasCompatibleUVs(regions[i], regions[j], sharedEdge)) {
                        continue;
                    }
                    
                    // Merge the regions
                    UVRegion merged = mergeTwo(regions[i], regions[j]);
                    
                    // Replace region i with merged result
                    regions[i] = merged;
                    
                    // Remove region j
                    regions.erase(regions.begin() + j);
                    
                    anyMerged = true;
                    
                    // Restart the search since indices changed
                    break;
                }
            }
            
            if (anyMerged) break;
        }
    }
    
    // Final cleanup
    cleanupDegenerateRegions();
    
    if (regions.size() > 1) {
        for (size_t i = 0; i < regions.size(); ++i) {
            float area = 0.0f;
            for (size_t j = 0; j < regions[i].positions.size(); ++j) {
                const vec2& p1 = regions[i].positions[j];
                const vec2& p2 = regions[i].positions[(j + 1) % regions[i].positions.size()];
                area += (p1.x * p2.y - p2.x * p1.y);
            }
            area = std::abs(area) * 0.5f;
        }
    }
}