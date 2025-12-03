#include "levels/levels.h"
#include "util/maths.h"
#include "audio/sfx_player.h"

Paper::Paper() : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{}

Paper::Paper(Mesh* mesh0, Mesh* mesh1, const std::vector<vec2>& region, std::pair<std::string, std::string> sideNames, std::pair<std::string, std::string> obstacleNames) : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    paperMeshes.first  = new PaperMesh(region, mesh0);
    auto obst = PaperMesh::obstacleTemplates[obstacleNames.first]();
    paperMeshes.first->regions.insert(paperMeshes.first->regions.begin(), obst.begin(), obst.end());

    paperMeshes.second = new PaperMesh(region, mesh1);
    obst = PaperMesh::obstacleTemplates[obstacleNames.second]();
    paperMeshes.second->regions.insert(paperMeshes.second->regions.begin(), obst.begin(), obst.end());

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
    audio::SFXPlayer::Get().Play("flip");
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

bool Paper::activateFold(const vec2& start) {
    activeFold = NULL_FOLD;

    // we push_back like a stack so the first fold we find is the top
    for (int i = static_cast<int>(folds.size()) - 1; i >= 0; i--) {
        if (folds[i].side != curSide) continue;

        if (folds[i].cover->contains(start)) {
            activeFold = i;
            return true;
        }
    }

    // check if we're clicking the paper if we're not clicking a fold
    PaperMesh* paperMesh = getPaperMesh();
    if (paperMesh->contains(start)) {
        activeFold = PAPER_FOLD;
        return true;
    }
    
    return false;
}

void Paper::deactivateFold() {
    activeFold = NULL_FOLD;
}

Paper::FoldGeometry Paper::validateFoldGeometry(const vec2& start, const vec2& end) {
    FoldGeometry geom;
    geom.isValid = false;
    
    if (activeFold == NULL_FOLD || glm::length2(start - end) < EPSILON) {
        return geom;
    }
    
    PaperMesh* paperMesh = getPaperMesh();
    
    // Check if start and end are inside the paper
    if (!paperMesh->contains(start) || !paperMesh->contains(end)) {
        return geom;
    }
    
    vec2 dx = end - start;
    geom.edgeIntersectPaper = paperMesh->getNearestEdgeIntersection(start, -dx);
    geom.nearEdgePointPaper = paperMesh->getNearestEdgePoint(start);
    geom.foldDir = end - geom.nearEdgePointPaper;
    geom.creasePos = 0.5f * (end + geom.nearEdgePointPaper);
    
    // Check if fold would go more than halfway across the paper
    float foldDistance = glm::length(geom.foldDir);
    if (foldDistance > EPSILON) {
        vec2 normalizedFoldDir = geom.foldDir / foldDistance;
        
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
            return geom;
        }
    }
    
    // Check if we're folding the paper directly (start is on the correct side)
    if (glm::dot(geom.edgeIntersectPaper - start, geom.nearEdgePointPaper - start) > 0) {
        geom.isValid = true;
    }
    
    return geom;
}

bool Paper::fold(const vec2& start, const vec2& end) {
    if (activeFold == NULL_FOLD || glm::length2(start - end) < EPSILON) return false;
    
    // Validate fold geometry and check if start/end are inside paper
    FoldGeometry geom = validateFoldGeometry(start, end);
    if (!geom.isValid) {
        return false;
    }

    // Paper is being folded directly
    Fold fold = Fold(start, curSide);
    bool check = fold.initialize(
        curSide == 0 ? paperMeshes : PaperMeshPair{ paperMeshes.second, paperMeshes.first }, 
        geom.creasePos, 
        geom.foldDir, 
        geom.edgeIntersectPaper
    );

    // stop fold if we run into trouble
    if (!check) return false;

    return pushFold(fold);
}

bool Paper::unfold(const vec2& pos) {
    activateFold(pos);
    bool check = popFold();
    if (check) {
        flip();
    };
    deactivateFold();
    return check;
}

bool Paper::pushFold(Fold& newFold) {
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
        return false;
    }

    check = paperCopy->paste(*newFold.cover);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        folds.pop_back();
        return false;
    }

    // Handle back side
    PaperMesh* backMesh = getBackPaperMesh();
    PaperMesh* backCopy = new PaperMesh(*backMesh);
    check = backCopy->cut(*newFold.backside);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        delete backCopy; backCopy = nullptr;
        folds.pop_back();
        return false;
    }

    // Calculate overhangs using the same method as previewFold:
    // overhang = coverRegion - uncutPaperRegion.
    const std::vector<vec2>& coverRegion = newFold.cover->region;
    Paths64 coverPath = makePaths64FromRegion(coverRegion);
    Paths64 paperPath = makePaths64FromRegion(paperMesh->region);
    Paths64 overhangPath;
    try {
        overhangPath = Difference(coverPath, paperPath, FillRule::NonZero);
    } catch (...) {
        std::cout << "pushFold: Exception while computing overhang Difference()" << std::endl;
        overhangPath.clear();
    }

    // Filter out degenerate / tiny overhang regions based on polygon area.
    auto polygonArea = [](const std::vector<vec2>& pts) -> float {
        if (pts.size() < 3) return 0.0f;
        float a = 0.0f;
        for (size_t i = 0; i < pts.size(); ++i) {
            const vec2& p0 = pts[i];
            const vec2& p1 = pts[(i + 1) % pts.size()];
            a += p0.x * p1.y - p1.x * p0.y;
        }
        return 0.5f * a;
    };

    const float OVERHANG_AREA_EPS = 1e-4f;
    bool hasOverhang = false;

    for (size_t pathIdx = 0; pathIdx < overhangPath.size(); ++pathIdx) {
        const Path64& path = overhangPath[pathIdx];
        if (path.size() < 3) continue;

        std::vector<vec2> pts;
        pts.reserve(path.size());
        for (const Point64& pt : path) {
            pts.emplace_back(
                static_cast<float>(pt.x / CLIPPER_SCALE),
                static_cast<float>(pt.y / CLIPPER_SCALE)
            );
        }

        float area = std::fabs(polygonArea(pts));
        if (area < OVERHANG_AREA_EPS) {
            std::cout << "pushFold: ignoring tiny overhang path, area=" << area << std::endl;
            continue;
        }

        hasOverhang = true;
        break;
    }

    if (hasOverhang) {
        std::cout << "pushFold: rejecting fold due to overhangs. "
                  << "numOverhangPaths=" << overhangPath.size() << std::endl;
        delete paperCopy; paperCopy = nullptr;
        delete backCopy;  backCopy  = nullptr;
        folds.pop_back();
        return false;
    }

    // Set front side region - no overhangs possible here anymore
    std::vector<vec2> frontRegion = newFold.cleanVerts;
    ensureCCW(frontRegion);
    paperCopy->region = frontRegion;
    paperCopy->pruneDups();

    // Set back side region - start with cleanFlipped
    std::vector<vec2> cleanFlipped;
    for (const auto& v : newFold.cleanVerts) cleanFlipped.push_back({ -v.x, v.y });
    std::reverse(cleanFlipped.begin(), cleanFlipped.end());
    ensureCCW(cleanFlipped);

    // Back side region: with overhangs disallowed, this is just the flipped clean region
    backCopy->region = cleanFlipped;
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
    paperMeshes.first->regenerateNavmesh();
    paperMeshes.second->regenerateNavmesh();
    sides.first->getBackground()->setMesh(paperMeshes.first->mesh);
    sides.second->getBackground()->setMesh(paperMeshes.second->mesh);
    regenerateWalls();

    // update active fold to be what we just made so we can hold onto it
    activeFold = folds.size() - 1;

    // DEBUG
    dotData();
    
    return true;
}

bool Paper::popFold() {
    if (activeFold < 0 || activeFold >= folds.size() || folds[activeFold].isCovered()) return false;

    // restore mesh from fold
    Fold& oldFold = folds[activeFold];
    PaperMesh* paperMesh = getPaperMesh();
    PaperMesh* backMesh = getBackPaperMesh();

    // Find insertion point for original folded vertices
    // We need to find where the crease start point is in the current region
    auto regionCopy = paperMesh->region;
    size_t insertPos = regionCopy.size(); // Default to end if not found
    bool foundCrease = false;
    
    for (size_t i = 0; i < regionCopy.size(); i++) {
        // Check if this vertex matches the crease start point
        if (glm::length2(oldFold.crease[0] - regionCopy[i]) < EPSILON) {
            insertPos = i + 1; // Insert after the crease start point
            foundCrease = true;
            break;
        }
    }

    if (!foundCrease) {
        std::cout << "popFold: Failed to find crease start point in region" << std::endl;
        return false;
    }

    // Insert original folded vertices after the crease start point
    // Insert in reverse order - each insert shifts subsequent elements, so inserting from end to start
    // maintains the correct final ordering
    for (auto it = oldFold.originalFoldedVerts.rbegin(); it != oldFold.originalFoldedVerts.rend(); ++it) {
        regionCopy.insert(regionCopy.begin() + insertPos, *it);
    }

    // Remove cover region from front paper
    bool check = paperMesh->cut(*oldFold.cover);
    if (!check) {
        std::cout << "popFold: Failed to cut cover region" << std::endl;
        return false;
    }
    
    // Restore underside region to front paper
    check = paperMesh->paste(*oldFold.underside);
    if (!check) {
        std::cout << "popFold: Failed to paste underside region" << std::endl;
        return false;
    }
    
    // Restore backside region to back paper
    check = backMesh->paste(*oldFold.backside);
    if (!check) {
        std::cout << "popFold: Failed to paste backside region" << std::endl;
        return false;
    }

    // Remove references TO the activeFold from other folds' holds
    for (Fold& fold : folds) {
        if (fold.holds.find(activeFold) != fold.holds.end()) {
            fold.holds.erase(activeFold);
        }
    }

    // apply copied region to paper mesh
    paperMesh->region = regionCopy;
    paperMesh->pruneDups();

    auto flippedRegionCopy = regionCopy;
    flipVecsHorizontal(flippedRegionCopy);
    std::reverse(flippedRegionCopy.begin(), flippedRegionCopy.end());
    ensureCCW(flippedRegionCopy);

    // apply copied region to back mesh
    backMesh->region = flippedRegionCopy;
    backMesh->pruneDups();

    // Clear all holds that the removed fold had on other folds
    oldFold.holds.clear();
    folds.erase(folds.begin() + activeFold);

    paperMeshes.first->regenerateMesh();
    paperMeshes.second->regenerateMesh();
    paperMeshes.first->regenerateNavmesh();
    paperMeshes.second->regenerateNavmesh();
    sides.first->getBackground()->setMesh(paperMeshes.first->mesh);
    sides.second->getBackground()->setMesh(paperMeshes.second->mesh);
    regenerateWalls();

    // DEBUG
    dotData();

    return true;
}

void Paper::regenerateWalls() {
    regenerateWalls(0);
    regenerateWalls(1);
}

void Paper::regenerateWalls(int side) {
    std::vector<vec2>& region = (side == 0) ? paperMeshes.first->region : paperMeshes.second->region;
    SingleSide* selectedSide = (side == 0) ? sides.first : sides.second;
    PaperMesh* selectedMesh = (side == 0) ? paperMeshes.first : paperMeshes.second;
    selectedSide->clearWalls();

    // create outer wall
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

    // create inner walls
    for (const auto& uvRegion : selectedMesh->regions) {
        if (uvRegion.isObstacle == false) continue;

        int regionSize = uvRegion.positions.size();
        for (int i = 0; i < regionSize; i++) {
            int j = (i + 1) % regionSize;
            auto data = connectSquare(uvRegion.positions[i], uvRegion.positions[j]);
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
            auto data = connectSquare(regionVerts[i], regionVerts[j], 0.025);
            
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
    // Basic guards – still reject obviously invalid drags
    if (activeFold == NULL_FOLD || glm::length2(start - end) < EPSILON) {
        return;
    }

    PaperMesh* paperMesh = getPaperMesh();
    if (!paperMesh->contains(start) || !paperMesh->contains(end)) {
        return;
    }

    // Recompute fold geometry here so we can distinguish "too far" cases
    // without rejecting them outright (unlike validateFoldGeometry)
    vec2 dx = end - start;
    vec2 edgeIntersectPaper = paperMesh->getNearestEdgeIntersection(start, -dx);
    vec2 nearEdgePointPaper = paperMesh->getNearestEdgePoint(start);
    vec2 foldDir = end - nearEdgePointPaper;
    vec2 creasePos = 0.5f * (end + nearEdgePointPaper);

    // Check if fold would go more than halfway across the paper
    bool tooFar = false;
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
            tooFar = true;
        }
    }

    // Still require that the fold direction/orientation is valid; otherwise skip preview
    if (glm::dot(edgeIntersectPaper - start, nearEdgePointPaper - start) <= 0) {
        return;
    }

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
        // Detect overhangs by subtracting the uncut paper region from the cover region:
        // overhang = coverRegion - paperRegion
        const std::vector<vec2>& coverRegion = tempFold.cover->region;
        Paths64 coverPath = makePaths64FromRegion(coverRegion);
        Paths64 paperPath = makePaths64FromRegion(paperMesh->region);

        Paths64 overhangPath;
        try {
            overhangPath = Difference(coverPath, paperPath, FillRule::NonZero);
        } catch (...) {
            overhangPath.clear();
        }

        // Filter out degenerate / tiny overhang regions based on polygon area.
        auto polygonArea = [](const std::vector<vec2>& pts) -> float {
            if (pts.size() < 3) return 0.0f;
            float a = 0.0f;
            for (size_t i = 0; i < pts.size(); ++i) {
                const vec2& p0 = pts[i];
                const vec2& p1 = pts[(i + 1) % pts.size()];
                a += p0.x * p1.y - p1.x * p0.y;
            }
            return 0.5f * a;
        };

        const float OVERHANG_AREA_EPS = 1e-4f;
        bool hasOverhang = false;

        // We will reuse non-degenerate overhang polygons when drawing red outlines.
        std::vector<std::vector<vec2>> nonDegenerateOverhangs;

        for (size_t pathIdx = 0; pathIdx < overhangPath.size(); ++pathIdx) {
            const Path64& path = overhangPath[pathIdx];
            if (path.size() < 3) continue;

            std::vector<vec2> pts;
            pts.reserve(path.size());
            for (const Point64& pt : path) {
                pts.emplace_back(
                    static_cast<float>(pt.x / CLIPPER_SCALE),
                    static_cast<float>(pt.y / CLIPPER_SCALE)
                );
            }

            float area = std::fabs(polygonArea(pts));
            if (area < OVERHANG_AREA_EPS) {
                // Very small sliver – treat as numerical noise.
                continue;
            }

            hasOverhang = true;
            nonDegenerateOverhangs.push_back(std::move(pts));
        }

        // Choose base material:
        //  - no overhangs, within extent -> green (valid)
        //  - no overhangs, too far      -> red (invalid extent)
        //  - any overhangs              -> yellow for contained part (with red overlays below)
        const char* baseMatName = nullptr;
        if (hasOverhang) {
            baseMatName = "yellow";
        } else if (tooFar) {
            baseMatName = "red";
        } else {
            baseMatName = "green";
        }

        // Visualize the cover region boundary
        for (int i = 0; i < static_cast<int>(coverRegion.size()); i++) {
            int j = (i + 1) % coverRegion.size();
            auto edgeData = connectSquare(coverRegion[i], coverRegion[j]);
            
            Node2D* edge = new Node2D(game->getScene(), {
                .mesh = game->getMesh("quad"),
                .material = game->getMaterial(baseMatName),
                .position = vec2{edgeData.first.x, edgeData.first.y},
                .rotation = edgeData.first.z,
                .scale = edgeData.second
            });
            edge->setLayer(0.95);
            regionNodes.push_back(edge);
        }

        // If we have overhangs, render their boundaries in red on top of the base preview.
        if (hasOverhang) {
            for (const auto& overhangPositionsRaw : nonDegenerateOverhangs) {
                if (overhangPositionsRaw.size() < 2) continue;

                std::vector<vec2> overhangPositions = overhangPositionsRaw;
                ensureCCW(overhangPositions);

                for (int i = 0; i < static_cast<int>(overhangPositions.size()); ++i) {
                    int j = (i + 1) % static_cast<int>(overhangPositions.size());
                    auto edgeData = connectSquare(overhangPositions[i], overhangPositions[j]);

                    Node2D* edge = new Node2D(game->getScene(), {
                        .mesh = game->getMesh("quad"),
                        .material = game->getMaterial("red"),
                        .position = vec2{edgeData.first.x, edgeData.first.y},
                        .rotation = edgeData.first.z,
                        .scale = edgeData.second
                    });
                    edge->setLayer(0.97f); // slightly above base preview
                    regionNodes.push_back(edge);
                }
            }
        }
    }
}

void Paper::updatePathing(vec2 playerPos) {
    SingleSide* side = (curSide == 0) ? sides.first : sides.second;
    PaperMesh* mesh = (curSide == 0) ? paperMeshes.first : paperMeshes.second;

    for (Enemy* enemy : side->getEnemies()) {
        std::vector<vec2> path;
        mesh->getPath(path, enemy->getPosition(), playerPos);
        enemy->setPath(path);
    }
}

void Paper::toData(std::vector<float>& out) {
    out.clear();

    std::vector<float> pageData;
    paperMeshes.first->toData(pageData);
    int i = 0;
    while (i < pageData.size()) {
        out.insert(out.end(), pageData.begin() + i + 0, pageData.end() + i + 5);
        out.push_back(0);
        out.push_back(0);
        out.push_back(1);

        i += 5;
    }

    pageData.clear();
    paperMeshes.second->toData(pageData);
    i = 0;
    while (i < pageData.size()) {
        out.push_back(pageData[i]);
        out.insert(out.end(), pageData.begin() + i + 1, pageData.end() + i + 5);
        out.push_back(0);
        out.push_back(0);
        out.push_back(1);

        i += 5;
    }
}