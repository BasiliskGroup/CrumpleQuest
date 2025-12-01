#include "levels/levels.h"
#include "util/maths.h"


Paper::Paper() : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{}

Paper::Paper(Mesh* mesh0, Mesh* mesh1, const std::vector<vec2>& region, std::pair<std::string, std::string> sideNames) : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    paperMeshes.first  = new PaperMesh(region, mesh0);
    paperMeshes.second = new PaperMesh(region, mesh1);

    sides.first  = SingleSide::templates[sideNames.first]();
    sides.second = SingleSide::templates[sideNames.second]();
}

Paper::Paper(const Paper& other)
    : curSide(other.curSide), 
      isOpen(other.isOpen),
      folds(other.folds),
      activeFold(other.activeFold),
      sides(nullptr, nullptr),
      paperMeshes(nullptr, nullptr)
{
    try {
        // Deep copy sides
        if (other.sides.first)
            sides.first = new SingleSide(*other.sides.first);
            
        if (other.sides.second)
            sides.second = new SingleSide(*other.sides.second);
        
        // Deep copy paperMeshes
        if (other.paperMeshes.first)
            paperMeshes.first = new PaperMesh(*other.paperMeshes.first);
            
        if (other.paperMeshes.second)
            paperMeshes.second = new PaperMesh(*other.paperMeshes.second);
    }
    catch (...) {
        // Clean up any partially constructed objects
        delete sides.first;
        delete sides.second;
        delete paperMeshes.first;
        delete paperMeshes.second;
        throw;
    }
}

// Move constructor
Paper::Paper(Paper&& other) noexcept
    : curSide(other.curSide),
      isOpen(other.isOpen),
      folds(std::move(other.folds)),
      activeFold(other.activeFold),
      sides(std::move(other.sides)),
      paperMeshes(std::move(other.paperMeshes))
{
    // Clear other
    other.sides = { nullptr, nullptr };
    other.paperMeshes = { nullptr, nullptr };
    other.activeFold = NULL_FOLD;
}

Paper::~Paper() {
    clear();
}

// Copy assignment - REMOVED noexcept since it can throw
Paper& Paper::operator=(const Paper& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for strong exception safety
    Paper temp(other);
    
    // Swap with temp
    std::swap(sides, temp.sides);
    std::swap(paperMeshes, temp.paperMeshes);
    std::swap(curSide, temp.curSide);
    std::swap(isOpen, temp.isOpen);
    std::swap(folds, temp.folds);
    std::swap(activeFold, temp.activeFold);
    
    // temp's destructor will clean up our old resources
    return *this;
}

// Move assignment
Paper& Paper::operator=(Paper&& other) noexcept {
    if (this == &other) return *this;
    
    clear();

    // Transfer ownership
    sides = std::move(other.sides);
    paperMeshes = std::move(other.paperMeshes);
    curSide = other.curSide;
    isOpen = other.isOpen;
    folds = std::move(other.folds);
    activeFold = other.activeFold;

    // Clear other
    other.sides = { nullptr, nullptr };
    other.paperMeshes = { nullptr, nullptr };
    other.activeFold = NULL_FOLD;

    return *this;
}

void Paper::flip() {
    curSide = curSide == 0 ? 1 : 0;
    dotData();
}

void Paper::open() {
    isOpen = true;
}

void Paper::clear() {
    delete sides.first;
    delete sides.second;
    sides = { nullptr, nullptr };
    
    delete paperMeshes.first;
    delete paperMeshes.second;
    paperMeshes = { nullptr, nullptr };
    
    isOpen = true;

    // Temporary
    regionNodes.clear();
}

Mesh* Paper::getMesh() { 
    PaperMesh* curMesh = getPaperMesh();
    if (curMesh == nullptr) return nullptr;
    return curMesh->mesh;
}

void Paper::activateFold(const vec2& start) {
    activeFold = NULL_FOLD;

    // we push_back like a stack so the first fold we find is the top
    for (int i = static_cast<int>(folds.size()) - 1; i >= 0; i--) {
        if (folds[i].side != curSide) continue;

        if (folds[i].cover->contains(start)) {
            activeFold = i;
            return;
        }
    }

    // check if we're clicking the paper if we're not clicking a fold
    PaperMesh* paperMesh = getPaperMesh();
    if (paperMesh->contains(start)) activeFold = PAPER_FOLD;
}

void Paper::deactivateFold() {
    activeFold = NULL_FOLD;
}

void Paper::fold(const vec2& start, const vec2& end) {
    if (activeFold == NULL_FOLD || glm::length2(start - end) < EPSILON) return;

    if (activeFold != PAPER_FOLD && folds[activeFold].holds.size() == 0) {
        popFold();
        activeFold = PAPER_FOLD; // Reset to paper fold so we recreate from scratch
    }
    
    // get starting geometry
    PaperMesh* paperMesh = getPaperMesh();
    vec2 dx = end - start;

    // paper intersections
    vec2 edgeIntersectPaper = paperMesh->getNearestEdgeIntersection(start, -dx);
    vec2 nearEdgePointPaper = paperMesh->getNearestEdgePoint(start);

    // fold intersections or use paper if we didn't click a fold
    vec2 edgeIntersectFold, nearEdgePointFold;
    Fold* clickedFold;
    if (activeFold == PAPER_FOLD) {
        edgeIntersectFold = edgeIntersectPaper;
        nearEdgePointFold = nearEdgePointPaper;
    } else {
        clickedFold = &folds[activeFold];
        edgeIntersectFold = clickedFold->cover->getNearestEdgeIntersection(start, -dx);
        nearEdgePointFold = clickedFold->cover->getNearestEdgePoint(start);
    }

    // variable to modify "start" position of fold, drop and replace
    vec2 foldDir = end - nearEdgePointPaper;
    vec2 creasePos = 0.5f * (end + nearEdgePointPaper);
    
    // Check if fold would go more than halfway across the paper
    float foldDistance = glm::length(foldDir);
    if (foldDistance > EPSILON) {
        vec2 normalizedFoldDir = foldDir / foldDistance;
        
        // Project all region vertices onto the fold direction to find paper extent
        float minProj = std::numeric_limits<float>::max();
        float maxProj = std::numeric_limits<float>::lowest();
        
        for (const vec2& v : paperMesh->region) {
            float proj = glm::dot(v, normalizedFoldDir);
            minProj = std::min(minProj, proj);
            maxProj = std::max(maxProj, proj);
        }
        
        float paperExtent = maxProj - minProj;
        
        // Reject fold if it exceeds half the paper extent in fold direction
        if (foldDistance > paperExtent * 0.5f) {
            std::cout << "Fold rejected: would exceed halfway across paper" << std::endl;
            return;
        }
    }
    
    // Paper is being folded directly
    if (glm::dot(edgeIntersectPaper - start, nearEdgePointPaper - start) > 0) {
        Fold fold = Fold(start, curSide);
        bool check = fold.initialize(
            curSide == 0 ? paperMeshes : PaperMeshPair{ paperMeshes.second, paperMeshes.first }, 
            creasePos, 
            foldDir, 
            edgeIntersectPaper
        );

        // stop fold if we run into trouble
        if (!check) return;

        pushFold(fold);
    }
}

void Paper::pushFold(Fold& newFold) {
    std::set<int> seen; // TODO implement cover cache
    int insertIndex = folds.size();

    // iterate from top to bottom since covering folds will come after covered
    for (int i = insertIndex - 1; i >= 0; i--) {
        Fold& fold = folds[i];
        
        // Check overlap with underside/backside based on which side the fold is on
        if (fold.underside->hasOverlap(fold.side == curSide ? *newFold.underside : *newFold.backside)) {
            fold.holds.insert(insertIndex);
            continue;
        }
        
        // For folds on the back side, also check if the crease line intersects their region
        if (fold.side != curSide) {
            // Check if the new fold's crease intersects the existing fold's backside region
            if (lineSegmentIntersectsPolygon(newFold.crease[0], newFold.crease[1], fold.backside->region)) {
                fold.holds.insert(insertIndex);
            }
        }
    }

    folds.push_back(newFold);

    // modify the mesh to accommodate new fold
    PaperMesh* paperMesh = getPaperMesh();
    PaperMesh* paperCopy = new PaperMesh(*paperMesh);

    bool check = paperCopy->cut(*newFold.underside);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        folds.pop_back();
        return;
    }

    check = paperCopy->paste(*newFold.cover);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        folds.pop_back();
        return;
    }

    // Handle back side
    PaperMesh* backMesh = getBackPaperMesh();
    PaperMesh* backCopy = new PaperMesh(*backMesh);
    check = backCopy->cut(*newFold.backside);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        delete backCopy; backCopy = nullptr;
        folds.pop_back();
        return;
    }

    // Calculate overhang: reflect cutVerts over crease to get fold landing positions
    std::vector<vec2> foldedCutVerts;
    for (const vec2& v : newFold.cutVerts) {
        foldedCutVerts.push_back(reflectPointOverLine(newFold.creasePos, newFold.creaseDir, v));
    }
    ensureCCW(foldedCutVerts);

    std::cout << "\n=== OVERHANG DEBUG ===" << std::endl;
    std::cout << "cutVerts size: " << newFold.cutVerts.size() << std::endl;
    std::cout << "foldedCutVerts size: " << foldedCutVerts.size() << std::endl;
    std::cout << "paperMesh->region size: " << paperMesh->region.size() << std::endl;

    // Find overhang: part of folded region outside paper boundary
    Paths64 foldedPath = makePaths64FromRegion(foldedCutVerts);
    Paths64 paperPath = makePaths64FromRegion(paperMesh->region);
    Paths64 overhangPath;
    
    std::cout << "foldedPath size: " << foldedPath.size() << std::endl;
    std::cout << "paperPath size: " << paperPath.size() << std::endl;
    
    try {
        overhangPath = Difference(foldedPath, paperPath, FillRule::NonZero);
        std::cout << "overhangPath size after Difference: " << overhangPath.size() << std::endl;
        if (!overhangPath.empty()) {
            std::cout << "overhangPath[0] points: " << overhangPath[0].size() << std::endl;
        }
    } catch (...) {
        std::cout << "Difference threw exception!" << std::endl;
        overhangPath.clear();
    }

    // Track ALL overhang regions for later region updates (multiple overhangs possible)
    std::vector<std::vector<vec2>> overhangFrontRegions;  // Front-side coordinates
    std::vector<std::vector<vec2>> overhangBackRegions;   // Back-side coordinates
    
    // Process ALL overhang paths, not just the largest one
    if (!overhangPath.empty()) {
        std::cout << "Processing " << overhangPath.size() << " overhang path(s)" << std::endl;
        
        for (size_t pathIdx = 0; pathIdx < overhangPath.size(); ++pathIdx) {
            const Path64& path = overhangPath[pathIdx];
            
            // Convert this path to vec2 positions
            std::vector<vec2> overhangPositions;
            overhangPositions.reserve(path.size());
            for (const Point64& pt : path) {
                overhangPositions.emplace_back(
                    static_cast<float>(pt.x / CLIPPER_SCALE),
                    static_cast<float>(pt.y / CLIPPER_SCALE)
                );
            }
            overhangPositions = simplifyCollinear(overhangPositions);
            
            std::cout << "  Overhang path " << pathIdx << " size: " << overhangPositions.size() << std::endl;
            
            if (overhangPositions.size() < 3) {
                std::cout << "    Skipping (too few vertices)" << std::endl;
                continue;
            }
            
            ensureCCW(overhangPositions);
            
            // Save overhang in front-side coordinates
            overhangFrontRegions.push_back(overhangPositions);
            std::cout << "    overhangFrontRegion[" << pathIdx << "] saved with " << overhangPositions.size() << " vertices" << std::endl;
            
            // Reflect overhang positions back to get source positions on original paper
            std::vector<vec2> overhangSourcePositions;
            for (const vec2& v : overhangPositions) {
                overhangSourcePositions.push_back(reflectPointOverLine(newFold.creasePos, newFold.creaseDir, v));
            }
            ensureCCW(overhangSourcePositions);
            
            // Create mesh at source positions - try front paper first, then back paper
            // (overhang might have originated from a fold on the other side)
            DyMesh* overhangSource = new DyMesh(overhangSourcePositions);
            PaperMesh* backMeshForCopy = getBackPaperMesh();
            
            bool copySuccess = overhangSource->copy(*paperMesh);
            if (!copySuccess && backMeshForCopy) {
                // Try the back mesh - the overhang might have come from the other side
                std::cout << "    overhangSource->copy from front FAILED, trying back mesh..." << std::endl;
                delete overhangSource;
                overhangSource = new DyMesh(overhangSourcePositions);
                copySuccess = overhangSource->copy(*backMeshForCopy);
            }
            
            if (copySuccess) {
                std::cout << "    overhangSource->copy succeeded" << std::endl;
                
                // Mirror over crease to get to overhang's front-side position (with UVs)
                DyMesh* overhangMirrored = overhangSource->mirror(newFold.creasePos, newFold.creaseDir);
                
                // Flip to back-side coordinates
                overhangMirrored->flipHorizontal();
                
                // Compute overhangBackRegion directly from overhangFrontRegion 
                // using the same transformation as cleanFlipped for consistency
                std::vector<vec2> overhangBackRegion;
                for (const auto& v : overhangPositions) {
                    overhangBackRegion.push_back({ -v.x, v.y });
                }
                std::reverse(overhangBackRegion.begin(), overhangBackRegion.end());
                overhangBackRegions.push_back(overhangBackRegion);
                std::cout << "    overhangBackRegion[" << pathIdx << "] saved with " << overhangBackRegion.size() << " vertices" << std::endl;
                
                // Paste onto back paper
                backCopy->paste(*overhangMirrored);
                
                delete overhangMirrored;
            } else {
                std::cout << "    overhangSource->copy FAILED (both front and back)" << std::endl;
            }
            delete overhangSource;
        }
    } else {
        std::cout << "No overhang detected (overhangPath empty)" << std::endl;
    }

    // Set front side region - union with ALL overhang regions
    std::vector<vec2> frontRegion = newFold.cleanVerts;
    ensureCCW(frontRegion);
    
    std::cout << "\n--- Front region processing ---" << std::endl;
    std::cout << "frontRegion (cleanVerts) size: " << frontRegion.size() << std::endl;
    std::cout << "overhangFrontRegions count: " << overhangFrontRegions.size() << std::endl;
    
    if (!overhangFrontRegions.empty()) {
        // Snap overhang vertices to frontRegion vertices to fix precision issues
        const float snapEps = 0.01f;
        for (size_t i = 0; i < overhangFrontRegions.size(); ++i) {
            for (vec2& ov : overhangFrontRegions[i]) {
                for (const vec2& fv : frontRegion) {
                    if (glm::length(ov - fv) < snapEps) {
                        ov = fv;
                        break;
                    }
                }
            }
        }
        
        // First, union all overhang regions together (they may touch each other)
        Paths64 allOverhangsPath;
        bool firstOverhang = true;
        
        for (size_t i = 0; i < overhangFrontRegions.size(); ++i) {
            std::vector<vec2>& overhangFrontRegion = overhangFrontRegions[i];
            if (overhangFrontRegion.size() < 3) continue;
            
            ensureCCW(overhangFrontRegion);
            Paths64 overhangFrontPath = makePaths64FromRegion(overhangFrontRegion);
            
            if (firstOverhang) {
                allOverhangsPath = overhangFrontPath;
                firstOverhang = false;
                std::cout << "  Starting with overhang " << i << std::endl;
            } else {
                try {
                    allOverhangsPath = Union(allOverhangsPath, overhangFrontPath, FillRule::NonZero);
                    std::cout << "  Added overhang " << i << ", total paths: " << allOverhangsPath.size() << std::endl;
                } catch (...) {
                    std::cout << "  Failed to union overhang " << i << std::endl;
                }
            }
        }
        
        // Now union the combined overhangs with frontRegion
        Paths64 frontPath = makePaths64FromRegion(frontRegion);
        Paths64 combinedPath;
        
        std::cout << "  Attempting final front union (frontRegion + all overhangs)..." << std::endl;
        
        try {
            combinedPath = Union(frontPath, allOverhangsPath, FillRule::NonZero);
            std::cout << "    Front union result paths: " << combinedPath.size() << std::endl;
        } catch (...) {
            std::cout << "    Front union threw exception" << std::endl;
            combinedPath = frontPath;
        }
        
        std::vector<vec2> combinedRegion = makeRegionFromPaths64(combinedPath);
        std::cout << "Final combinedRegion size: " << combinedRegion.size() << std::endl;
        
        if (combinedRegion.size() >= 3) {
            combinedRegion = simplifyCollinear(combinedRegion);
            ensureCCW(combinedRegion);
            paperCopy->region = combinedRegion;
            std::cout << "Front region set to combined (" << combinedRegion.size() << " verts)" << std::endl;
        } else {
            paperCopy->region = frontRegion;
            std::cout << "Combined too small, using frontRegion" << std::endl;
        }
    } else {
        paperCopy->region = frontRegion;
        std::cout << "No overhang for front, using frontRegion" << std::endl;
    }
    paperCopy->pruneDups();
    std::cout << "Final front region size: " << paperCopy->region.size() << std::endl;

    // Set back side region - start with cleanFlipped
    std::vector<vec2> cleanFlipped;
    for (const auto& v : newFold.cleanVerts) cleanFlipped.push_back({ -v.x, v.y });
    std::reverse(cleanFlipped.begin(), cleanFlipped.end());
    ensureCCW(cleanFlipped);

    std::cout << "\n--- Back region processing ---" << std::endl;
    std::cout << "cleanFlipped size: " << cleanFlipped.size() << std::endl;
    std::cout << "overhangBackRegions count: " << overhangBackRegions.size() << std::endl;

    // Union ALL overhang regions with the back region
    if (!overhangBackRegions.empty()) {
        // Snap overhang vertices to cleanFlipped vertices to fix precision issues
        const float snapEps = 0.01f;
        for (size_t i = 0; i < overhangBackRegions.size(); ++i) {
            for (vec2& ov : overhangBackRegions[i]) {
                for (const vec2& cv : cleanFlipped) {
                    if (glm::length(ov - cv) < snapEps) {
                        ov = cv;
                        break;
                    }
                }
            }
        }
        
        // First, union all overhang regions together (they may touch each other)
        Paths64 allOverhangsPath;
        bool firstOverhang = true;
        
        for (size_t i = 0; i < overhangBackRegions.size(); ++i) {
            std::vector<vec2>& overhangBackRegion = overhangBackRegions[i];
            if (overhangBackRegion.size() < 3) continue;
            
            ensureCCW(overhangBackRegion);
            Paths64 overhangBackPath = makePaths64FromRegion(overhangBackRegion);
            
            if (firstOverhang) {
                allOverhangsPath = overhangBackPath;
                firstOverhang = false;
                std::cout << "  Starting with overhang " << i << std::endl;
            } else {
                try {
                    allOverhangsPath = Union(allOverhangsPath, overhangBackPath, FillRule::NonZero);
                    std::cout << "  Added overhang " << i << ", total paths: " << allOverhangsPath.size() << std::endl;
                } catch (...) {
                    std::cout << "  Failed to union overhang " << i << std::endl;
                }
            }
        }
        
        // Now union the combined overhangs with cleanFlipped
        Paths64 cleanFlippedPath = makePaths64FromRegion(cleanFlipped);
        Paths64 combinedPath;
        
        std::cout << "  Attempting final back union (cleanFlipped + all overhangs)..." << std::endl;
        
        try {
            combinedPath = Union(cleanFlippedPath, allOverhangsPath, FillRule::NonZero);
            std::cout << "    Back union result paths: " << combinedPath.size() << std::endl;
        } catch (...) {
            std::cout << "    Back union threw exception" << std::endl;
            combinedPath = cleanFlippedPath;
        }
        
        std::vector<vec2> combinedRegion = makeRegionFromPaths64(combinedPath);
        std::cout << "Final combinedRegion size: " << combinedRegion.size() << std::endl;
        
        if (combinedRegion.size() >= 3) {
            combinedRegion = simplifyCollinear(combinedRegion);
            ensureCCW(combinedRegion);
            backCopy->region = combinedRegion;
            std::cout << "Back region set to combined (" << combinedRegion.size() << " verts)" << std::endl;
        } else {
            backCopy->region = cleanFlipped;
            std::cout << "Combined too small, using cleanFlipped" << std::endl;
        }
    } else {
        backCopy->region = cleanFlipped;
        std::cout << "No overhang for back, using cleanFlipped" << std::endl;
    }
    std::cout << "Final back region size: " << backCopy->region.size() << std::endl;
    std::cout << "=== END OVERHANG DEBUG ===\n" << std::endl;
    
    backCopy->pruneDups();

    // swap meshes with cut
    delete paperMesh;
    delete backMesh;
    
    if (curSide == 0) {
        this->paperMeshes = { paperCopy, backCopy };
    } else {
        this->paperMeshes = { backCopy, paperCopy };
    }

    paperMeshes.first->regenerateMesh();
    paperMeshes.second->regenerateMesh();
    paperMeshes.first->mergeAllRegions();
    paperMeshes.second->mergeAllRegions();
    sides.first->getBackground()->setMesh(paperMeshes.first->mesh);
    sides.second->getBackground()->setMesh(paperMeshes.second->mesh);
    regenerateWalls();

    // update active fold to be what we just made so we can hold onto it
    activeFold = folds.size() - 1;

    // DEBUG
    dotData();
}

void Paper::popFold() {
    if (activeFold < 0 || activeFold >= folds.size()) return;

    // restore mesh from fold
    Fold& oldFold = folds[activeFold];
    PaperMesh* paperMesh = getPaperMesh();
    PaperMesh* backMesh = getBackPaperMesh();

    // When unfolding, we need to:
    // 1. Remove the cover region (which is currently on top of the folded area)
    // 2. Restore the underside region (which was folded under)
    // 3. Restore the backside region (which was on the back)
    
    // Remove cover region from front paper
    bool check = paperMesh->cut(*oldFold.cover);
    if (!check) {
        std::cout << "popFold: Failed to cut cover region" << std::endl;
        return;
    }
    
    // Restore underside region to front paper
    check = paperMesh->paste(*oldFold.underside);
    if (!check) {
        std::cout << "popFold: Failed to paste underside region" << std::endl;
        return;
    }
    
    // Restore backside region to back paper
    check = backMesh->paste(*oldFold.backside);
    if (!check) {
        std::cout << "popFold: Failed to paste backside region" << std::endl;
        return;
    }

    // Remove references TO the activeFold from other folds' holds
    for (Fold& fold : folds) {
        if (fold.holds.find(activeFold) != fold.holds.end()) {
            fold.holds.erase(activeFold);
        }
    }
    
    // Clear all holds that the removed fold had on other folds
    oldFold.holds.clear();

    // active fold should be near to the back so this isn't the worst
    folds.erase(folds.begin() + activeFold);

    paperMeshes.first->regenerateMesh();
    paperMeshes.second->regenerateMesh();
    paperMeshes.first->mergeAllRegions();
    paperMeshes.second->mergeAllRegions();
    sides.first->getBackground()->setMesh(paperMeshes.first->mesh);
    sides.second->getBackground()->setMesh(paperMeshes.second->mesh);
    regenerateWalls();

    // DEBUG
    dotData();
}

void Paper::regenerateWalls() {
    regenerateWalls(0);
    regenerateWalls(1);
}

void Paper::regenerateWalls(int side) {
    std::vector<vec2>& region = (side == 0) ? paperMeshes.first->region : paperMeshes.second->region;
    SingleSide* selectedSide = (side == 0) ? sides.first : sides.second;
    selectedSide->clearWalls();

    for (int i = 0; i < region.size(); i++) {
        int j = (i + 1) % region.size();
        auto data = connectSquare(region[i], region[j]);
        selectedSide->addWall(new Node2D(selectedSide->getScene(), { 
            .mesh = game->getMesh("quad"), 
            .material = game->getMaterial("knight"), 
            .position = vec2{data.first.x, data.first.y}, 
            .rotation = data.first.z, 
            .scale = data.second,
            .collider = selectedSide->getCollider("quad"),
            .density = -1
        }));
    }
}

void Paper::dotData() {
    // Clear existing debug nodes
    for (uint i = 0; i < regionNodes.size(); i++) {
        delete regionNodes[i];
    }
    regionNodes.clear();

    PaperMesh* paperMesh = getPaperMesh();
    
    // Render outer boundary
    std::vector<vec2>& outerRegion = paperMesh->region;
    for (int i = 0; i < outerRegion.size(); i++) {
        auto& r = outerRegion[i];
        Node2D* n = new Node2D(game->getScene(), { 
            .mesh = game->getMesh("quad"), 
            .material = game->getMaterial("man"), 
            .position = r, 
            .scale = {0.1 + 0.025 * i, 0.1 + 0.025 * i} 
        });
        n->setLayer(0.9);
        regionNodes.push_back(n);
    }

    // Render each UV region's boundary as walls/edges
    for (size_t regionIdx = 0; regionIdx < paperMesh->regions.size(); regionIdx++) {
        const UVRegion& uvRegion = paperMesh->regions[regionIdx];
        const std::vector<vec2>& regionVerts = uvRegion.positions;
        
        // Create walls for this region's boundary
        for (int i = 0; i < regionVerts.size(); i++) {
            int j = (i + 1) % regionVerts.size();
            auto data = connectSquare(regionVerts[i], regionVerts[j]);
            
            Node2D* wall = new Node2D(game->getScene(), { 
                .mesh = game->getMesh("quad"), 
                .material = game->getMaterial("lightGrey"),  // Or use a different material per region
                .position = vec2{data.first.x, data.first.y}, 
                .rotation = data.first.z, 
                .scale = data.second
            });
            wall->setLayer(0.85);
            regionNodes.push_back(wall);
        }
    }
}

void Paper::previewFold(const vec2& start, const vec2& end) {
    if (activeFold == NULL_FOLD || glm::length2(start - end) < EPSILON) {
        return;
    }
    
    PaperMesh* paperMesh = getPaperMesh();
    vec2 dx = end - start;
    vec2 edgeIntersectPaper = paperMesh->getNearestEdgeIntersection(start, -dx);
    vec2 nearEdgePointPaper = paperMesh->getNearestEdgePoint(start);
    vec2 foldDir = end - nearEdgePointPaper;
    vec2 creasePos = 0.5f * (end + nearEdgePointPaper);
    
    // Check if fold would be valid (same checks as fold function)
    float foldDistance = glm::length(foldDir);
    if (foldDistance > EPSILON) {
        vec2 normalizedFoldDir = foldDir / foldDistance;
        float minProj = std::numeric_limits<float>::max();
        float maxProj = std::numeric_limits<float>::lowest();
        for (const vec2& v : paperMesh->region) {
            float proj = glm::dot(v, normalizedFoldDir);
            minProj = std::min(minProj, proj);
            maxProj = std::max(maxProj, proj);
        }
        float paperExtent = maxProj - minProj;
        if (foldDistance > paperExtent * 0.5f) {
            return;
        }
    }
    
    if (glm::dot(edgeIntersectPaper - start, nearEdgePointPaper - start) > 0) {
        Fold tempFold(start, curSide);
        // Try to initialize - even if it fails, cover might still be created
        bool check = tempFold.initialize(
            curSide == 0 ? paperMeshes : PaperMeshPair{ paperMeshes.second, paperMeshes.first },
            creasePos,
            foldDir,
            edgeIntersectPaper
        );
        
        // Show cover even if initialize returned false, as long as cover is valid
        if (tempFold.cover != nullptr && !tempFold.cover->region.empty() && tempFold.cover->region.size() >= 3) {
            // Visualize the cover region boundary
            const std::vector<vec2>& coverRegion = tempFold.cover->region;
            for (int i = 0; i < coverRegion.size(); i++) {
                int j = (i + 1) % coverRegion.size();
                auto edgeData = connectSquare(coverRegion[i], coverRegion[j]);
                
                Node2D* edge = new Node2D(game->getScene(), {
                    .mesh = game->getMesh("quad"),
                    .material = game->getMaterial("test"),
                    .position = vec2{edgeData.first.x, edgeData.first.y},
                    .rotation = edgeData.first.z,
                    .scale = edgeData.second
                });
                edge->setLayer(0.95);
                regionNodes.push_back(edge);
            }
        }
    }
}
