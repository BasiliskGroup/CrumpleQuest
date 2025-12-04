#include "levels/levels.h"
#include "util/maths.h"
#include "audio/sfx_player.h"
#include "util/clipper_helper.h"

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
    std::cout << "[Paper::Paper] Created " << obst.size() << " obstacles from template '" << obstacleNames.first << "'" << std::endl;
    for (size_t i = 0; i < obst.size(); i++) {
        std::cout << "[Paper::Paper] Obstacle " << i << " has isObstacle=" << obst[i].isObstacle << std::endl;
    }
    paperMeshes.first->regions.insert(paperMeshes.first->regions.begin(), obst.begin(), obst.end());
    std::cout << "[Paper::Paper] After insertion, paperMeshes.first has " << paperMeshes.first->regions.size() << " regions" << std::endl;
    paperMeshes.first->regenerateNavmesh();  // Regenerate after adding obstacles

    paperMeshes.second = new PaperMesh(region, mesh1);
    obst = PaperMesh::obstacleTemplates[obstacleNames.second]();
    paperMeshes.second->regions.insert(paperMeshes.second->regions.begin(), obst.begin(), obst.end());
    paperMeshes.second->regenerateNavmesh();  // Regenerate after adding obstacles

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
    // Clear preview outline by restoring debug visualization
    dotData();
}

Paper::FoldGeometry Paper::validateFoldGeometry(const vec2& start, const vec2& end) {
    FoldGeometry geom;
    geom.isValid = false;
    geom.playerInFold = false;
    
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
        
        // Check if player is in the fold underside region (the part that gets folded underneath)
        SingleSide* currentSide = getSingleSide();
        if (currentSide && currentSide->getPlayerNode()) {
            vec2 playerPos = currentSide->getPlayerNode()->getPosition();
            
            // Create a temporary fold to check if player is in the underside
            Fold tempFold(start, curSide);
            bool check = tempFold.initialize(
                curSide == 0 ? paperMeshes : PaperMeshPair{ paperMeshes.second, paperMeshes.first },
                geom.creasePos,
                geom.foldDir,
                geom.edgeIntersectPaper
            );
            
            if (check && tempFold.underside != nullptr) {
                geom.playerInFold = tempFold.underside->contains(playerPos);
            }
        }
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
    
    // Reject fold if player is in the fold cover region
    if (geom.playerInFold) {
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
    
    // Before popping the fold, check which enemies are in the fold underside region
    // We need to save this information before popFold() erases the fold
    std::vector<Enemy*> enemiesToMove;
    SingleSide* currentSide = getSingleSide();
    SingleSide* otherSide = getBackSide();
    
    // Save crease information before the fold is popped
    vec2 creaseStart, creaseEnd;
    bool hasCrease = false;
    
    if (activeFold >= 0 && activeFold < folds.size()) {
        Fold& activeFoldRef = folds[activeFold];
        if (activeFoldRef.underside != nullptr) {
            // Check all enemies on the current side
            auto& enemies = currentSide->getEnemies();
            for (Enemy* enemy : enemies) {
                if (enemy == nullptr || enemy->isDead()) continue;
                vec2 enemyPos = enemy->getPosition();
                if (activeFoldRef.underside->contains(enemyPos)) {
                    enemiesToMove.push_back(enemy);
                }
            }
        }
        
        // Save crease information
        if (activeFoldRef.crease.size() == 2) {
            creaseStart = activeFoldRef.crease[0];
            creaseEnd = activeFoldRef.crease[1];
            hasCrease = true;
        }
    }
    
    bool check = popFold();
    if (check) {
        // Move player (player position is already reflected in popFold)
        vec2 playerPos = getSingleSide()->getPlayerNode()->getPosition();
        flip();
        playerPos = { -playerPos.x, playerPos.y };
        getSingleSide()->getPlayerNode()->setPosition(playerPos);
        
        // Move enemies that were in the fold
        for (Enemy* enemy : enemiesToMove) {
            if (enemy == nullptr || enemy->isDead()) continue;
            
            // Get the enemy's position before adopting (since adoptEnemy will delete the old node)
            vec2 enemyPos = enemy->getPosition();
            
            // Reflect the position over the crease line (same as player in popFold)
            // This undoes the reflection that happened when the fold was created
            vec2 reflectedPos = enemyPos;
            if (hasCrease) {
                vec2 creaseDir = creaseEnd - creaseStart;
                float creaseLen = glm::length(creaseDir);
                if (creaseLen > EPSILON) {
                    reflectedPos = reflectPointOverLine(creaseStart, creaseDir, enemyPos);
                }
            }
            
            // Adopt the enemy to the other side
            otherSide->adoptEnemy(enemy, currentSide);
            
            // Flip the position to the other side (same as player transformation after flip())
            vec2 newPos = { -reflectedPos.x, reflectedPos.y };
            enemy->setPosition(newPos);
        }
    }
    deactivateFold();
    return check;
}

bool Paper::pushFold(Fold& newFold) {
    // Safety check: reject fold if player is in the fold underside region (the part that gets folded underneath)
    SingleSide* currentSide = getSingleSide();
    SingleSide* backSide = getBackSide();
    
    if (currentSide && currentSide->getPlayerNode() && newFold.underside != nullptr) {
        vec2 playerPos = currentSide->getPlayerNode()->getPosition();
        if (newFold.underside->contains(playerPos)) {
            std::cout << "pushFold: rejecting fold - player is in fold underside region" << std::endl;
            return false;
        }
    }
    
    // Check for enemies on the current side that are in the fold underside region
    // Push them to the nearest edge of the cover region (excluding crease line)
    if (currentSide && newFold.underside != nullptr && newFold.cover != nullptr && newFold.crease.size() == 2) {
        auto& enemies = currentSide->getEnemies();
        for (Enemy* enemy : enemies) {
            if (enemy == nullptr || enemy->isDead()) continue;
            vec2 enemyPos = enemy->getPosition();
            if (newFold.underside->contains(enemyPos)) {
                // Find nearest point on non-crease edges of cover region
                size_t edgeIndex = 0;
                vec2 edgePoint = findNearestPointOnNonCreaseEdges(enemyPos, newFold.cover->region, newFold.crease, edgeIndex);
                
                // Calculate inward normal from the edge (pointing into the cover region)
                // For CCW polygon, edge goes from region[i] to region[i+1]
                // Inward normal is to the left: (-edgeDir.y, edgeDir.x) normalized
                const vec2& edgeStart = newFold.cover->region[edgeIndex];
                const vec2& edgeEnd = newFold.cover->region[(edgeIndex + 1) % newFold.cover->region.size()];
                vec2 edgeDir = edgeEnd - edgeStart;
                float edgeLen = glm::length(edgeDir);
                if (edgeLen > EPSILON) {
                    // Inward normal for CCW polygon (pointing left/into polygon)
                    vec2 inwardNormal = vec2(-edgeDir.y, edgeDir.x) / edgeLen;
                    
                    // Move position 0.25 units inward from the edge
                    vec2 newPos = edgePoint + inwardNormal * 0.25f;
                    
                    // Calculate direction vector for velocity
                    vec2 pushDir = newPos - enemyPos;
                    float pushDist = glm::length(pushDir);
                    if (pushDist > EPSILON) {
                        // Normalize and scale to ~10
                        pushDir = glm::normalize(pushDir) * 10.0f;
                        
                        // Move enemy to new position (slightly inside the cover region)
                        enemy->setPosition(newPos);
                        
                        // Set velocity in push direction
                        enemy->setVelocity(vec3(pushDir.x, pushDir.y, 0.0f));
                    }
                } else {
                    // Fallback if edge is degenerate
                    vec2 newPos = edgePoint;
                    vec2 pushDir = newPos - enemyPos;
                    float pushDist = glm::length(pushDir);
                    if (pushDist > EPSILON) {
                        pushDir = glm::normalize(pushDir) * 10.0f;
                        enemy->setPosition(newPos);
                        enemy->setVelocity(vec3(pushDir.x, pushDir.y, 0.0f));
                    }
                }
            }
        }
    }
    
    // Before pushing the fold, check which enemies on the back side are in the fold backside region
    // We need to save this information before the fold is pushed (which modifies the meshes)
    std::vector<Enemy*> enemiesToMove;
    if (backSide && newFold.backside != nullptr) {
        // Check all enemies on the back side
        auto& enemies = backSide->getEnemies();
        for (Enemy* enemy : enemies) {
            if (enemy == nullptr || enemy->isDead()) continue;
            vec2 enemyPos = enemy->getPosition();
            // The backside region is in the back side's coordinate system (x is negated)
            // So we can check directly if the enemy position is in the backside region
            if (newFold.backside->contains(enemyPos)) {
                enemiesToMove.push_back(enemy);
            }
        }
    }
    
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
    frontRegion = simplifyCollinear(frontRegion);
    paperCopy->region = frontRegion;
    paperCopy->pruneDups();

    // Set back side region - start with cleanFlipped
    std::vector<vec2> cleanFlipped;
    for (const auto& v : newFold.cleanVerts) cleanFlipped.push_back({ -v.x, v.y });
    std::reverse(cleanFlipped.begin(), cleanFlipped.end());
    ensureCCW(cleanFlipped);
    cleanFlipped = simplifyCollinear(cleanFlipped);

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

    // Move enemies that were on the back side in the fold region to the current side
    for (Enemy* enemy : enemiesToMove) {
        if (enemy == nullptr || enemy->isDead()) continue;
        
        // Get the enemy's position before adopting (since adoptEnemy will delete the old node)
        vec2 enemyPos = enemy->getPosition();
        
        // Adopt the enemy to the current side (where the player is)
        currentSide->adoptEnemy(enemy, backSide);
        
        // Reflect the position over the crease line
        vec2 newPos;
        if (newFold.crease.size() == 2) {
            vec2 creaseStart = newFold.crease[0];
            vec2 creaseEnd = newFold.crease[1];
            vec2 creaseDir = creaseEnd - creaseStart;
            float creaseLen = glm::length(creaseDir);
            
            if (creaseLen > EPSILON) {
                // First flip to the front side
                vec2 flippedPos = { -enemyPos.x, enemyPos.y };
                // Then reflect over the crease line
                newPos = reflectPointOverLine(creaseStart, creaseDir, flippedPos);
            } else {
                // Fallback to simple flip if crease is invalid
                newPos = { -enemyPos.x, enemyPos.y };
            }
        } else {
            // Fallback to simple flip if no crease information
            newPos = { -enemyPos.x, enemyPos.y };
        }
        enemy->setPosition(newPos);
    }

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
    
    // Simplify collinear vertices after inserting
    regionCopy = simplifyCollinear(regionCopy);

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
    paperMesh->region = simplifyCollinear(paperMesh->region);
    paperMesh->pruneDups();

    auto flippedRegionCopy = regionCopy;
    flipVecsHorizontal(flippedRegionCopy);
    std::reverse(flippedRegionCopy.begin(), flippedRegionCopy.end());
    ensureCCW(flippedRegionCopy);
    flippedRegionCopy = simplifyCollinear(flippedRegionCopy);

    // apply copied region to back mesh
    backMesh->region = flippedRegionCopy;
    backMesh->region = simplifyCollinear(backMesh->region);
    backMesh->pruneDups();

    // Save crease information before erasing the fold (since oldFold reference becomes invalid)
    vec2 creaseStart = oldFold.crease[0];
    vec2 creaseEnd = oldFold.crease[1];
    
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

    // Reflect player position over the crease line
    SingleSide* currentSide = getSingleSide();
    if (currentSide && currentSide->getPlayerNode()) {
        vec2 playerPos = currentSide->getPlayerNode()->getPosition();
        
        // Compute crease direction from crease endpoints
        vec2 creaseDir = creaseEnd - creaseStart;
        float creaseLen = glm::length(creaseDir);
        
        // Only reflect if crease direction is valid
        if (creaseLen > EPSILON) {
            vec2 reflectedPos = reflectPointOverLine(creaseStart, creaseDir, playerPos);
            currentSide->getPlayerNode()->setPosition(reflectedPos);
        }
    }

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

    // Check if player is in fold using validateFoldGeometry
    FoldGeometry geom = validateFoldGeometry(start, end);
    bool playerInFold = geom.playerInFold;

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
        //  - no overhangs, within extent, player not in fold -> green (valid)
        //  - no overhangs, too far      -> red (invalid extent)
        //  - player in fold              -> yellow (invalid - would fold over player, but show yellow with red square)
        //  - any overhangs              -> yellow for contained part (with red overlays below)
        const char* baseMatName = nullptr;
        if (hasOverhang) {
            baseMatName = "yellow";
        } else if (tooFar) {
            baseMatName = "red";
        } else if (playerInFold) {
            baseMatName = "yellow";
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
        
        // If player is in fold, draw a red square around the player
        if (playerInFold) {
            SingleSide* currentSide = getSingleSide();
            if (currentSide && currentSide->getPlayerNode()) {
                vec2 playerPos = currentSide->getPlayerNode()->getPosition();
                float halfSide = 0.375f; // half of 0.75
                
                // Create 4 corners of the square
                vec2 corners[4] = {
                    {playerPos.x - halfSide, playerPos.y + halfSide}, // top-left
                    {playerPos.x + halfSide, playerPos.y + halfSide}, // top-right
                    {playerPos.x + halfSide, playerPos.y - halfSide}, // bottom-right
                    {playerPos.x - halfSide, playerPos.y - halfSide}  // bottom-left
                };
                
                // Draw 4 edges to form the square
                for (int i = 0; i < 4; i++) {
                    int j = (i + 1) % 4;
                    auto edgeData = connectSquare(corners[i], corners[j]);
                    
                    Node2D* edge = new Node2D(game->getScene(), {
                        .mesh = game->getMesh("quad"),
                        .material = game->getMaterial("red"),
                        .position = vec2{edgeData.first.x, edgeData.first.y},
                        .rotation = edgeData.first.z,
                        .scale = edgeData.second
                    });
                    edge->setLayer(0.98f); // above base preview
                    regionNodes.push_back(edge);
                }
            }
        }
    }
}

void Paper::padCornerWaypoints(std::vector<vec2>& path, float padding) {
    if (path.size() < 3) return; // Need at least start, corner, end
    
    // Process corner waypoints (skip first and last)
    for (size_t i = 1; i < path.size() - 1; i++) {
        vec2& corner = path[i];
        const vec2& prev = path[i - 1];
        const vec2& next = path[i + 1];
        
        // Calculate directions from corner to previous and next waypoints
        vec2 toPrev = prev - corner;
        vec2 toNext = next - corner;
        
        float lenToPrev = glm::length(toPrev);
        float lenToNext = glm::length(toNext);
        
        // Skip if directions are degenerate
        if (lenToPrev < EPSILON || lenToNext < EPSILON) continue;
        
        // Normalize directions
        toPrev /= lenToPrev;
        toNext /= lenToNext;
        
        // Calculate the angle bisector - average of the two normalized directions
        // This points away from the corner, opening up the turn
        vec2 bisector = toPrev + toNext;
        float bisectorLen = glm::length(bisector);
        
        // If the two directions are opposite (collinear path), skip padding
        if (bisectorLen < EPSILON) continue;
        
        bisector /= bisectorLen;
        
        // Offset the corner waypoint outward along the bisector by the padding distance
        corner += bisector * padding;
    }
}

bool Paper::edgeFallsOnCrease(const vec2& edgeStart, const vec2& edgeEnd, const Vec2Pair& crease, float epsilon) {
    // Check if both endpoints of the edge are very close to the crease line segment
    float distStart = distancePointToEdge(crease[0], crease[1], edgeStart);
    float distEnd = distancePointToEdge(crease[0], crease[1], edgeEnd);
    
    return (distStart < epsilon && distEnd < epsilon);
}

vec2 Paper::findNearestPointOnNonCreaseEdges(const vec2& point, const std::vector<vec2>& coverRegion, const Vec2Pair& crease, size_t& outEdgeIndex) {
    if (coverRegion.size() < 2) {
        outEdgeIndex = 0;
        return point;
    }
    
    float bestDistSq = std::numeric_limits<float>::max();
    vec2 bestPoint = point;
    outEdgeIndex = 0;
    
    for (size_t i = 0; i < coverRegion.size(); i++) {
        const vec2& edgeStart = coverRegion[i];
        const vec2& edgeEnd = coverRegion[(i + 1) % coverRegion.size()];
        
        // Skip edges that fall on the crease line
        if (edgeFallsOnCrease(edgeStart, edgeEnd, crease)) {
            continue;
        }
        
        // Find nearest point on this edge
        vec2 candidate = nearestPointOnEdgeToPoint(edgeStart, edgeEnd, point);
        float distSq = glm::length2(candidate - point);
        
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestPoint = candidate;
            outEdgeIndex = i;
        }
    }
    
    return bestPoint;
}

void Paper::updatePathing(vec2 playerPos) {
    SingleSide* side = (curSide == 0) ? sides.first : sides.second;
    PaperMesh* mesh = (curSide == 0) ? paperMeshes.first : paperMeshes.second;

    for (Enemy* enemy : side->getEnemies()) {
        std::vector<vec2> path;
        // Pass enemy radius + a bit extra for portal padding (1.2x radius for safety margin)
        float padding = enemy->getRadius() * 1.2f;
        mesh->getPath(path, enemy->getPosition(), playerPos, padding);
        
        // Add padding to corner waypoints so enemies don't get caught
        padCornerWaypoints(path, enemy->getRadius());
        
        enemy->setPath(path);
    }
}

void Paper::toData(std::vector<float>& out) {
    out.clear();
    std::vector<float> out2;

    std::vector<float> pageData;
    std::vector<vec2> region = paperMeshes.first->region;
    std::pair<vec2, vec2> aabb = paperMeshes.first->getAABB();
    
    // Triangulate region
    std::vector<std::vector<std::array<double, 2>>> polygon;
    polygon.emplace_back();
    polygon[0].reserve(region.size());

    for (const vec2& v : region) {
        polygon[0].push_back({{static_cast<double>(v.x), static_cast<double>(v.y)}});
    }

    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    vec2 aabbMin = aabb.first;   // bottom-left
    vec2 aabbMax = aabb.second;  // top-right
    vec2 size = aabbMax - aabbMin;  // positive size vector
    
    // Guard against division by zero
    if (size.x < EPSILON || size.y < EPSILON || region.size() < 3) {
        // Return empty mesh if region is degenerate
        return;
    }

    // Generate triangles
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            uint32_t idx = indices[i + j];
            if (idx >= region.size()) continue; // Safety check
            const vec2& pos = region[idx];
            
            // Interpolate UV in AABB: normalize position to [0, 1] range
            vec2 uv = (pos - aabbMin) / size;

            // First side: left half of framebuffer [0, 0.5] x [0, 1]
            out.push_back(pos.x / 10.0f);
            out.push_back(pos.y / 10.0f);
            out.push_back(0.001f);
            out.push_back(uv.x * 0.5f);  // Map to left half: [0, 0.5]
            out.push_back(uv.y);
            out.push_back(0.0f);
            out.push_back(0.0f);
            out.push_back(1.0f);

            // Second side: right half of framebuffer [0.5, 1] x [0, 1]
            out2.push_back(-pos.x / 10.0f);
            out2.push_back(pos.y / 10.0f);
            out2.push_back(-0.001f);
            out2.push_back((1.0f + uv.x) * 0.5f);  // Map to right half: [0.5, 1]
            out2.push_back(uv.y);
            out2.push_back(0.0f);
            out2.push_back(0.0f);
            out2.push_back(1.0f);
        }
    }

    out.insert(out.begin(), out2.begin(), out2.end());
}