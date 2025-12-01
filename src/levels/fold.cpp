#include "levels/levels.h"

Paper::Fold::Fold(const vec2& start, int side) :
    underside(nullptr),
    backside(nullptr),
    cover(nullptr),
    holds(),
    start(start),
    side(side),
    crease(),
    creasePos(),
    creaseDir(),
    cleanVerts(),
    cutVerts()
{}

bool Paper::Fold::initialize(PaperMeshPair meshes, const vec2& inCreasePos, const vec2& foldDir, const vec2& edgeIntersectPaper) {
    // precalculate reference geometry
    float midDot = glm::dot(inCreasePos, foldDir);
    vec2 inCreaseDir = { foldDir.y, -foldDir.x };
    vec2 searchStart = edgeIntersectPaper; // reference TODO remove?

    // Store crease info for later use
    this->creasePos = inCreasePos;
    this->creaseDir = inCreaseDir;

    // get new fold geometry by intersecting crease with paper
    auto indexBounds = meshes.first->getVertexRangeBelowThreshold(foldDir, midDot, searchStart);

    // determine crease intersection with paper
    vec2 foldStart, foldEnd;
    bool leftCheck = meshes.first->getEdgeIntersection(indexBounds.first - 1, inCreasePos, inCreaseDir, foldStart); // -1 since ccw
    bool rightCheck = meshes.first->getEdgeIntersection(indexBounds.second, inCreasePos, inCreaseDir, foldEnd);

    bool check = leftCheck & rightCheck;
    if (!check) {
        std::cout << "Fold crease could not find intersection" << std::endl;
        return false;
    }
    
    crease = { foldStart, foldEnd };

    // create cut region
    this->cutVerts = { foldStart };
    meshes.first->addRangeInside(this->cutVerts, indexBounds);
    this->cutVerts.push_back(foldEnd);

    meshes.first->addRangeOutside(cleanVerts, indexBounds.first, indexBounds.second); // cleaning, used in Paper
    cleanVerts.push_back(foldStart);
    cleanVerts.push_back(foldEnd);
    
    // sample from back side of paper
    std::vector<vec2> backVerts = this->cutVerts;
    flipVecsHorizontal(backVerts);
    ensureCCW(backVerts);
    
    backside = new DyMesh(backVerts);
    check = backside->copy(*meshes.second);  // Get UVs from back of paper
    if (!check) {
        std::cout << "Failed to copy negative-cut" << std::endl;
        return false;
    }

    // find the cover region on front side
    DyMesh backFlipped = DyMesh(*backside);
    backFlipped.flipHorizontal(); // flip before mirroring?
    cover = backFlipped.mirror(inCreasePos, inCreaseDir);

    // TODO vertex proximity check ...

    // find sandwiched overed region
    std::vector<vec2> undersideVerts = this->cutVerts;
    meshes.first->reflectVerticesOverLine(undersideVerts, indexBounds.first, indexBounds.second, inCreasePos, inCreaseDir);
    
    underside = new DyMesh(undersideVerts);
    check = underside->copy(*meshes.first);
    if (!check) { 
        std::cout << "Failed to copy underlayer" << std::endl; 
        return false; 
    }

    return true;
}

// rule of 5
Paper::Fold::~Fold() {
    delete underside; underside = nullptr;
    delete backside; backside = nullptr;
    delete cover; cover = nullptr;
}

Paper::Fold::Fold(const Fold& other) :
    underside(other.underside ? new DyMesh(*other.underside) : nullptr),
    backside(other.backside ? new DyMesh(*other.backside) : nullptr),
    cover(other.cover ? new DyMesh(*other.cover) : nullptr),
    holds(other.holds),
    start(other.start),
    side(other.side),
    crease(other.crease),
    creasePos(other.creasePos),
    creaseDir(other.creaseDir),
    cleanVerts(other.cleanVerts),
    cutVerts(other.cutVerts)
{}

Paper::Fold::Fold(Fold&& other) noexcept :
    underside(other.underside),
    backside(other.backside),
    cover(other.cover),
    holds(std::move(other.holds)),
    start(other.start),
    side(other.side),
    crease(other.crease),
    creasePos(other.creasePos),
    creaseDir(other.creaseDir),
    cleanVerts(std::move(other.cleanVerts)),
    cutVerts(std::move(other.cutVerts))
{
    other.underside = nullptr;
    other.backside = nullptr;
    other.cover = nullptr;
}

Paper::Fold& Paper::Fold::operator=(const Fold& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    Fold temp(other);
    
    delete underside;
    delete cover;
    delete backside;
    
    underside = temp.underside;
    backside = temp.backside;
    cover = temp.cover;
    holds = std::move(temp.holds);
    start = temp.start;
    side = temp.side;
    crease = temp.crease;
    creasePos = temp.creasePos;
    creaseDir = temp.creaseDir;
    cleanVerts = std::move(temp.cleanVerts);
    cutVerts = std::move(temp.cutVerts);
    
    temp.underside = nullptr;
    temp.backside = nullptr;
    temp.cover = nullptr;
    
    return *this;
}

Paper::Fold& Paper::Fold::operator=(Fold&& other) noexcept {
    if (this == &other) return *this;
    
    delete underside;
    delete backside;
    delete cover;
    
    underside = other.underside;
    backside = other.backside;
    cover = other.cover;
    holds = std::move(other.holds);
    start = other.start;
    side = other.side;
    crease = other.crease;
    creasePos = other.creasePos;
    creaseDir = other.creaseDir;
    cleanVerts = std::move(other.cleanVerts);
    cutVerts = std::move(other.cutVerts);
    
    other.underside = nullptr;
    other.backside = nullptr;
    other.cover = nullptr;
    
    return *this;
}