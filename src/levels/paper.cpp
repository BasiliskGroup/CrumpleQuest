#include "levels/levels.h"

Paper::Paper() : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{}

Paper::Paper(Mesh* mesh0, Mesh* mesh1, const std::vector<vec2>& region) : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    paperMeshes.first = new PaperMesh(region, mesh0);
    paperMeshes.second = new PaperMesh(region, mesh1);
}

Paper::Paper(const Paper& other)
    : curSide(other.curSide), 
      isOpen(other.isOpen),
      folds(other.folds),
      activeFold(other.activeFold),
      sides({ nullptr, nullptr }),
      paperMeshes({ nullptr, nullptr })
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
    for (uint i = 0; i < regionNodes.size(); i++) {
        delete regionNodes[i];
    }
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

    // Reactivate an old fold and reset its progress
    // For now, we restore the fold to refold, maybe find more efficient solution layer
    if (activeFold != PAPER_FOLD 
     && glm::length2(nearEdgePointFold - nearEdgePointPaper) < EPSILON 
     && clickedFold->holds.empty()
    ) {
        popFold();
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
    } else {
        std::cout << "unfold" << std::endl;
    }
}

void Paper::pushFold(Fold& newFold) {
    std::set<int> seen; // TODO implement cover cache
    int insertIndex = folds.size();

    // iterate from top to bottom since covering folds will come after covered
    for (int i = insertIndex - 1; i >= 0; i--) {
        // prevents swapping unfold layers
        Fold& fold = folds[i];
        if (fold.underside->hasOverlap(*newFold.underside) == false) continue;
        fold.holds.insert(insertIndex);
    }

    folds.push_back(newFold);

    // modify the mesh to accommodate new fold
    // copy mesh so we can quit fold is shit goes wrong
    PaperMesh* paperMesh = getPaperMesh();
    PaperMesh* paperCopy = new PaperMesh(*paperMesh);

    bool check = paperCopy->cut(*newFold.underside);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        return;
    }

    check = paperCopy->paste(*newFold.cover);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        return;
    }

    // check for erroneous points
    // auto& reg = newFold.underside->region;
    // auto& cA = newFold.crease[0];
    // auto& cB = newFold.crease[1];

    // std::vector<vec2> insides;
    // insides.reserve(reg.size());

    // for (const vec2& v : reg) {
    //     // skip if equal to either crease point
    //     if (glm::all(glm::epsilonEqual(v, cA, 1e-6f))) continue;
    //     if (glm::all(glm::epsilonEqual(v, cB, 1e-6f))) continue;

    //     insides.push_back(v);
    // }

    // TODO make a copy first
    PaperMesh* backMesh = getBackPaperMesh();
    PaperMesh* backCopy = new PaperMesh(*backMesh);
    check = backCopy->cut(*newFold.backside);
    if (!check) {
        delete paperCopy; paperCopy = nullptr;
        delete backCopy; backCopy = nullptr;
        return;
    }

    // all tests passed
    paperCopy->keepOnly(newFold.cleanVerts);
    paperCopy->pruneDups();
    paperCopy->removeDataOutside();

    backCopy->pruneDups();
    backCopy->removeDataOutside();

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

    // TODO add holds to under folds


    // DEBUG
    dotData();
}

void Paper::popFold() {
    if (activeFold < 0 || activeFold >= folds.size()) return;

    // restore mesh from fold
    Fold& oldFold = folds[activeFold];
    PaperMesh* paperMesh = getPaperMesh();

    // TODO, add checks on these
    paperMesh->cut(*oldFold.underside);
    paperMesh->paste(*oldFold.underside);
    getBackPaperMesh()->paste(*oldFold.backside);

    for (Fold& fold : folds) {
        if (fold.holds.find(activeFold) != fold.holds.end()) {
            fold.holds.erase(activeFold);
        }
    }

    // active fold should be near to the back so this isn't the worst
    folds.erase(folds.begin() + activeFold);

    paperMeshes.first->regenerateMesh();
    paperMeshes.second->regenerateMesh();

    // DEBUG
    dotData();
}

void Paper::dotData() {
    for (uint i = 0; i < regionNodes.size(); i++) {
        delete regionNodes[i];
    }
    regionNodes.clear();

    auto& printRegion = getPaperMesh()->region; // folds.back().underside->region;
    for (int i = 0; i < printRegion.size(); i++) {
        auto& r = printRegion[i];
        Node2D* n = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .position=r, .scale={0.1 + 0.025 * i, 0.1 + 0.025 * i} });
        n->setLayer(0.9);
        regionNodes.push_back(n);
    }

    for (const Tri& d : getPaperMesh()->data) {
        for (const Vert& v : d.verts) {
            Node2D* n = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .position=v.pos, .scale={0.05, 0.05} });
            n->setLayer(0.9);
            regionNodes.push_back(n);
        }
    }
}