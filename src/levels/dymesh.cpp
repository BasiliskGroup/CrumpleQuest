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
            positions.push_back({verts[i + 0], verts[i + 1]});
            uvs.push_back({verts[i + 3], verts[i + 4]});
            i += 5;
        }
        
        regions.emplace_back(positions, uvs);
    }
}

DyMesh::DyMesh(const std::vector<vec2>& region) : Edger(region), regions() {
    ensureCCW(this->region);
    
    if (region.size() < 3) return;
    
    // Create a single region with simple UV mapping
    std::vector<vec2> uvs;
    uvs.reserve(region.size());
    
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
    
    // Map positions to [0,1] UV space
    for (const vec2& p : region) {
        vec2 uv = (p - minPos) / maxDim;
        uvs.push_back(uv);
    }
    
    regions.emplace_back(region, uvs);
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

    if (sol.size() > 1) {
        std::cout << "Polygon was cut into multiple pieces" << std::endl;
        return false;
    }
    
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

            // Sample UVs from original region for each new vertex
            std::vector<vec2> clippedUVs;
            clippedUVs.reserve(clippedPositions.size());
            
            for (const vec2& p : clippedPositions) {
                clippedUVs.push_back(uvReg.sampleUV(p));
            }

            newRegions.emplace_back(clippedPositions, clippedUVs);
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
        regions.clear();
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
        
        // Keep UVs as-is (or flip if needed)
        mirroredRegions.emplace_back(mirroredPositions, uvReg.uvs);
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
                const vec2& uv = uvReg.uvs[idx];

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
            const vec2& uv = regions[i].uvs[j];
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

    // Find indices in r1 for the shared edge vertices
    int r1_idx1 = -1, r1_idx2 = -1;
    for (size_t i = 0; i < r1.positions.size(); ++i) {
        if (glm::length(r1.positions[i] - sharedEdge[0]) < eps) r1_idx1 = i;
        if (glm::length(r1.positions[i] - sharedEdge[1]) < eps) r1_idx2 = i;
    }

    // Find indices in r2 for the shared edge vertices
    int r2_idx1 = -1, r2_idx2 = -1;
    for (size_t i = 0; i < r2.positions.size(); ++i) {
        if (glm::length(r2.positions[i] - sharedEdge[0]) < eps) r2_idx1 = i;
        if (glm::length(r2.positions[i] - sharedEdge[1]) < eps) r2_idx2 = i;
    }

    if (r1_idx1 == -1 || r1_idx2 == -1 || r2_idx1 == -1 || r2_idx2 == -1) {
        return false;
    }

    // Check if UVs match at shared vertices
    bool uv1Match = glm::length(r1.uvs[r1_idx1] - r2.uvs[r2_idx1]) < eps;
    bool uv2Match = glm::length(r1.uvs[r1_idx2] - r2.uvs[r2_idx2]) < eps;

    return uv1Match && uv2Match;
}

bool DyMesh::canMergeRegions(const UVRegion& r1, const UVRegion& r2, std::vector<vec2>& sharedEdge) const {
    if (!hasSharedEdge(r1, r2, sharedEdge)) return false;
    if (!hasCompatibleUVs(r1, r2, sharedEdge)) return false;
    return true;
}

void DyMesh::cleanupDegenerateRegions() {
    const float eps = 1e-5f;
    
    std::cout << "\n--- Cleaning up degenerate regions ---" << std::endl;
    std::cout << "Initial region count: " << regions.size() << std::endl;
    
    for (size_t i = 0; i < regions.size(); ) {
        std::cout << "Checking region " << i << " (" << regions[i].positions.size() << " vertices)..." << std::endl;
        
        // Remove duplicate vertices
        std::vector<vec2> cleanedPos;
        std::vector<vec2> cleanedUV;
        
        for (size_t j = 0; j < regions[i].positions.size(); ++j) {
            bool isDup = false;
            for (size_t k = 0; k < cleanedPos.size(); ++k) {
                if (glm::length(regions[i].positions[j] - cleanedPos[k]) < eps) {
                    std::cout << "  Removing duplicate at (" << regions[i].positions[j].x 
                             << ", " << regions[i].positions[j].y << ")" << std::endl;
                    isDup = true;
                    break;
                }
            }
            if (!isDup) {
                cleanedPos.push_back(regions[i].positions[j]);
                cleanedUV.push_back(regions[i].uvs[j]);
            }
        }
        
        // Check if region is degenerate after cleanup
        if (cleanedPos.size() < 3) {
            std::cout << "  Region " << i << " is degenerate (only " << cleanedPos.size() 
                     << " unique vertices), removing" << std::endl;
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
        
        // Only remove truly tiny artifacts (< 0.1 square units)
        // This is about 1% of a 10x10 square, truly microscopic
        const float minArea = 0.1f;
        if (area < minArea) {
            std::cout << "  Region " << i << " has area " << area << " (microscopic artifact), removing" << std::endl;
            regions.erase(regions.begin() + i);
            continue;
        }
        
        // Update region if we removed duplicates
        if (cleanedPos.size() != regions[i].positions.size()) {
            std::cout << "  Region " << i << " cleaned: " << regions[i].positions.size() 
                     << " -> " << cleanedPos.size() << " vertices" << std::endl;
            regions[i].positions = cleanedPos;
            regions[i].uvs = cleanedUV;
        } else {
            std::cout << "  Region " << i << " is valid (area=" << area << ")" << std::endl;
        }
        
        ++i;
    }
    
    std::cout << "Final region count after cleanup: " << regions.size() << std::endl;
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

    // Sample UVs from original regions for each vertex of merged region
    std::vector<vec2> mergedUVs;
    mergedUVs.reserve(mergedPositions.size());

    for (const vec2& pos : mergedPositions) {
        // Try to sample from r1 first, then r2
        if (r1.contains(pos, 1e-5f)) {
            mergedUVs.push_back(r1.sampleUV(pos));
        } else if (r2.contains(pos, 1e-5f)) {
            mergedUVs.push_back(r2.sampleUV(pos));
        } else {
            // Fallback: pick closer region
            float dist1 = r1.distance(pos);
            float dist2 = r2.distance(pos);
            mergedUVs.push_back(dist1 < dist2 ? r1.sampleUV(pos) : r2.sampleUV(pos));
        }
    }

    return UVRegion(mergedPositions, mergedUVs);
}

void DyMesh::mergeAllRegions() {
    std::cout << "\n===== Starting mergeAllRegions =====" << std::endl;
    std::cout << "Initial region count: " << regions.size() << std::endl;
    
    if (regions.size() < 2) {
        std::cout << "mergeAllRegions: Too few regions, skipping" << std::endl;
        return;
    }
    
    // First cleanup pass
    cleanupDegenerateRegions();
    
    if (regions.size() < 2) {
        std::cout << "mergeAllRegions: Too few regions after cleanup, done" << std::endl;
        return;
    }
    
    bool anyMerged = true;
    int iteration = 0;
    
    while (anyMerged && regions.size() > 1) {
        anyMerged = false;
        iteration++;
        std::cout << "\n--- Merge Iteration " << iteration << " ---" << std::endl;
        std::cout << "Current region count: " << regions.size() << std::endl;
        
        // Try to merge each pair of regions
        for (size_t i = 0; i < regions.size(); ++i) {
            for (size_t j = i + 1; j < regions.size(); ++j) {
                std::vector<vec2> sharedEdge;
                
                // Check if regions share an edge
                bool hasShared = hasSharedEdge(regions[i], regions[j], sharedEdge);
                
                if (hasShared) {
                    // Check if UVs are compatible on the shared edge
                    if (!hasCompatibleUVs(regions[i], regions[j], sharedEdge)) {
                        std::cout << "  Regions " << i << " and " << j << " share edge but have incompatible UVs" << std::endl;
                        
                        // Print UV details for debugging
                        std::cout << "    Region " << i << " edge UVs: ";
                        for (const vec2& pt : sharedEdge) {
                            for (size_t k = 0; k < regions[i].positions.size(); ++k) {
                                if (glm::length(regions[i].positions[k] - pt) < 1e-5f) {
                                    std::cout << "(" << regions[i].uvs[k].x << "," << regions[i].uvs[k].y << ") ";
                                    break;
                                }
                            }
                        }
                        std::cout << std::endl;
                        
                        std::cout << "    Region " << j << " edge UVs: ";
                        for (const vec2& pt : sharedEdge) {
                            for (size_t k = 0; k < regions[j].positions.size(); ++k) {
                                if (glm::length(regions[j].positions[k] - pt) < 1e-5f) {
                                    std::cout << "(" << regions[j].uvs[k].x << "," << regions[j].uvs[k].y << ") ";
                                    break;
                                }
                            }
                        }
                        std::cout << std::endl;
                        
                        continue;
                    }
                    
                    std::cout << "  Merging regions " << i << " (size=" << regions[i].positions.size() 
                             << ") and " << j << " (size=" << regions[j].positions.size() << ")" << std::endl;
                    
                    // Merge the regions
                    UVRegion merged = mergeTwo(regions[i], regions[j]);
                    
                    // Replace region i with merged result
                    regions[i] = merged;
                    
                    // Remove region j
                    regions.erase(regions.begin() + j);
                    
                    anyMerged = true;
                    std::cout << "  Successfully merged! New region count: " << regions.size() << std::endl;
                    
                    // Restart the search since indices changed
                    break;
                }
            }
            
            if (anyMerged) break;
        }
    }
    
    // Final cleanup
    cleanupDegenerateRegions();
    
    std::cout << "\n===== Finished mergeAllRegions =====" << std::endl;
    std::cout << "Final region count: " << regions.size() << std::endl;
    
    if (regions.size() > 1) {
        std::cout << "\nNOTE: " << regions.size() << " separate regions remain." << std::endl;
        std::cout << "Analyzing remaining regions:" << std::endl;
        
        for (size_t i = 0; i < regions.size(); ++i) {
            float area = 0.0f;
            for (size_t j = 0; j < regions[i].positions.size(); ++j) {
                const vec2& p1 = regions[i].positions[j];
                const vec2& p2 = regions[i].positions[(j + 1) % regions[i].positions.size()];
                area += (p1.x * p2.y - p2.x * p1.y);
            }
            area = std::abs(area) * 0.5f;
            
            std::cout << "  Region " << i << ": " << regions[i].positions.size() 
                     << " vertices, area=" << area << std::endl;
            std::cout << "    Positions: ";
            for (const vec2& p : regions[i].positions) {
                std::cout << "(" << p.x << "," << p.y << ") ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "\nPossible reasons:" << std::endl;
        std::cout << "  - Regions have incompatible UVs (texture seams)" << std::endl;
        std::cout << "  - Regions don't share edges (separated/interior regions)" << std::endl;
    }
}