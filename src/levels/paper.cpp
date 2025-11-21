#include "levels/levels.h"

Paper::Paper() : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{}

Paper::Paper(Mesh* mesh, const std::vector<vec2>& region) : 
    curSide(0), 
    isOpen(false),
    activeFold(NULL_FOLD),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    std::vector<Point64> pointRegion;
    pointRegion.reserve(region.size());
    for (const vec2& r : region) {
        pointRegion.push_back(vec2ToPoint64(r));
    }

    paperMeshes.first = new PaperMesh(pointRegion, mesh);
    paperMeshes.second = new PaperMesh(pointRegion, mesh);
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

void Paper::fold(const vec2& mouseStart, const vec2& mouseEnd) {
    if (activeFold == NULL_FOLD || glm::length2(mouseStart - mouseEnd) < EPSILON) return;

    std::cout << "\n=== PAPER FOLD DEBUG ===" << std::endl;
    std::cout << "MouseStart: (" << mouseStart.x << ", " << mouseStart.y << ")" << std::endl;
    std::cout << "MouseEnd: (" << mouseEnd.x << ", " << mouseEnd.y << ")" << std::endl;

    Point64 start = vec2ToPoint64(mouseStart);
    Point64 end = vec2ToPoint64(mouseEnd);
    
    std::cout << "Start (Point64): (" << start.x << ", " << start.y << ")" << std::endl;
    std::cout << "End (Point64): (" << end.x << ", " << end.y << ")" << std::endl;
    
    PaperMesh* paperMesh = getPaperMesh();
    std::cout << "Paper region vertices: " << paperMesh->region.size() << std::endl;
    for (size_t i = 0; i < paperMesh->region.size(); i++) {
        std::cout << "  V" << i << ": (" << paperMesh->region[i].x << ", " << paperMesh->region[i].y << ")" << std::endl;
    }

    Point64 edgeIntersectPaper;
    bool check = paperMesh->getNearestEdgeIntersection(start, end, edgeIntersectPaper);
    
    if (!check) {
        std::cout << "No intersection with paper edge" << std::endl;
        return;
    }
    
    std::cout << "EdgeIntersectPaper: (" << edgeIntersectPaper.x << ", " << edgeIntersectPaper.y << ")" << std::endl;

    // Calculate fold direction
    Point64 foldDirection = end - start;
    std::cout << "FoldDirection: (" << foldDirection.x << ", " << foldDirection.y << ")" << std::endl;
    
    // Crease line is perpendicular to fold direction
    Point64 creasePerpendicular = {-foldDirection.y, foldDirection.x};
    std::cout << "CreasePerpendicular (unscaled): (" << creasePerpendicular.x << ", " << creasePerpendicular.y << ")" << std::endl;
    
    // Scale the perpendicular
    double perpLen = std::sqrt(static_cast<double>(length264(creasePerpendicular)));
    std::cout << "Perpendicular length: " << perpLen << std::endl;
    
    if (perpLen < 1e-10) {
        std::cout << "Invalid fold direction (perpLen too small)" << std::endl;
        return;
    }
    
    int64_t targetLength = 20000000;
    
    Point64 scaledPerp = {
        static_cast<int64_t>((creasePerpendicular.x * targetLength) / perpLen),
        static_cast<int64_t>((creasePerpendicular.y * targetLength) / perpLen)
    };
    
    std::cout << "ScaledPerp: (" << scaledPerp.x << ", " << scaledPerp.y << ")" << std::endl;
    
    Point64 crease0 = {
        edgeIntersectPaper.x - scaledPerp.x,
        edgeIntersectPaper.y - scaledPerp.y
    };
    
    Point64 crease1 = {
        edgeIntersectPaper.x + scaledPerp.x,
        edgeIntersectPaper.y + scaledPerp.y
    };
    
    std::cout << "Crease0: (" << crease0.x << ", " << crease0.y << ")" << std::endl;
    std::cout << "Crease1: (" << crease1.x << ", " << crease1.y << ")" << std::endl;
    std::cout << "Crease line length: " << std::sqrt(static_cast<double>(length264(crease1 - crease0))) << std::endl;

    if (activeFold == PAPER_FOLD) {
        try {
            Fold fold = Fold(paperMesh, crease0, crease1, edgeIntersectPaper, start, curSide);
            pushFold(fold);
            
            // ... rest of code
        } catch (const std::exception& e) {
            std::cout << "Fold failed: " << e.what() << std::endl;
        }
    }
    
    std::cout << "=== END PAPER FOLD DEBUG ===\n" << std::endl;
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
    PaperMesh* paperMesh = getPaperMesh();
    paperMesh->cut(*newFold.underside);
    paperMesh->paste(*newFold.cover);
}

void Paper::popFold() {
    if (activeFold < 0 || activeFold >= folds.size()) return;

    // restore mesh from fold
    Fold& oldFold = folds[activeFold];
    PaperMesh* paperMesh = getPaperMesh();
    paperMesh->cut(*oldFold.underside);
    paperMesh->paste(*oldFold.underside);

    for (Fold& fold : folds) {
        if (fold.holds.find(activeFold) != fold.holds.end()) {
            fold.holds.erase(activeFold);
        }
    }

    // active fold should be near to the back so this isn't the worst
    folds.erase(folds.begin() + activeFold);
}

// How to make a new fold on a piece of paper (no fold extensions)
// find start and end points of fold direction
// find when the fold line pointing from end to start meets the paper (fold interscetion direction)
// find the closes point on the paper to the start of the fold (origin of fold)
// using these, determine if we are folding or unfolding. if unfolding, return. 
// create a new fold object
// - find crease line
// - find intersection of crease line and paper
// - copy the part to be removed from the piece of paper & its UVs (UVs will be changed later but this is good for testing)
// - reflect over crease line to get the fold cover
// - construct the hidden part underneath the fold using the union geometry from removal and reflected parts
// cut the underneath out of the paper (we cant see it any more)
// paste the cover onto the paper (this is what we see)