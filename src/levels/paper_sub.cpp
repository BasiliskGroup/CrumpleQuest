#include "levels/levels.h"

// ------------------------------------------------------------
// PaperMesh
// ------------------------------------------------------------

Paper::PaperMesh::PaperMesh(const std::vector<vec2> verts, Mesh* mesh) : DyMesh(verts, mesh), mesh(nullptr) {
    std::vector<float> data; 
    toData(data);
    this->mesh = new Mesh(data);
}

Paper::PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
}

Paper::PaperMesh::PaperMesh(const PaperMesh& other) : DyMesh(other.region, other.data), mesh(nullptr) {
    std::vector<float> data;
    toData(data);
    mesh = new Mesh(data);
}

Paper::PaperMesh::PaperMesh(PaperMesh&& other) noexcept : DyMesh(std::move(other.region), std::move(other.data)), mesh(other.mesh) {
    other.mesh = nullptr;
}

Paper::PaperMesh& Paper::PaperMesh::operator=(const PaperMesh& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    PaperMesh temp(other);
    
    delete mesh;
    mesh = temp.mesh;
    region = std::move(temp.region);
    data = std::move(temp.data);
    temp.mesh = nullptr;
    
    return *this;
}

// Move assignment
Paper::PaperMesh& Paper::PaperMesh::operator=(PaperMesh&& other) noexcept {
    if (this == &other) return *this;
    
    delete mesh;
    
    region = std::move(other.region);
    data = std::move(other.data);
    mesh = other.mesh;
    other.mesh = nullptr;
    
    return *this;
}

void Paper::PaperMesh::regenerateMesh() {
    Mesh* oldPaperMesh = mesh;
    std::vector<float> newMeshData;
    toData(newMeshData);
    mesh = new Mesh(newMeshData);
    delete oldPaperMesh;
}

// ------------------------------------------------------------
// Fold
// ------------------------------------------------------------

// called when creating a fold
Paper::Fold::Fold(const vec2& start, int side) :
    underside(nullptr),
    cover(nullptr),
    holds(),
    start(start),
    side(side),
    crease()
{}

bool Paper::Fold::initialize(PaperMesh* paperMesh, const vec2& creasePos, const vec2& foldDir, const vec2& edgeIntersectPaper) {
    // precalculate reference geometry
    float midDot = glm::dot(creasePos, foldDir);
    vec2 creaseDir = { foldDir.y, -foldDir.x };
    vec2 searchStart = edgeIntersectPaper; // reference TODO remove?

    // get new fold geometry by intersecting crease with paper
    auto indexBounds = paperMesh->getVertexRangeBelowThreshold(foldDir, midDot, searchStart);

    // determine crease intersection with paper
    vec2 foldStart, foldEnd;
    bool leftCheck = paperMesh->getEdgeIntersection(indexBounds.first - 1, creasePos, creaseDir, foldStart); // -1 since ccw
    bool rightCheck = paperMesh->getEdgeIntersection(indexBounds.second, creasePos, creaseDir, foldEnd);

    // TODO switch to graceful error handling for more reliable game play
    bool check = leftCheck & rightCheck;
    if (!check) {
        std::cout << "Fold crease could not find intersection" << std::endl;
        return false;
    }

    crease = { foldStart, foldEnd };

    // create cut dymesh (negative part of fold)
    // TODO, check if these are always wound the correct direction
    std::vector<vec2> cutVerts = { foldStart };
    paperMesh->addRangeInside(cutVerts, indexBounds);
    cutVerts.push_back(foldEnd);

    // we store the cut so that it can be accessed by the paper outside, don't midify the PaperMesh in the Fold constructor
    DyMesh cut = DyMesh(cutVerts);
    check = cut.copy(*paperMesh); // TODO cut from back side of page later
    if (!check) {
        std::cout << "Failed to copy negative-cut" << std::endl;
        return false;
    }

    // folding the paper back mirrors it from its normal position
    cover = cut.mirror(creasePos, creaseDir);

    // finish creating the underside
    // TODO check if these are CCW
    paperMesh->reflectVerticesOverLine(cutVerts, indexBounds.first, indexBounds.second, creasePos, creaseDir);
    underside = new DyMesh(cutVerts);
    check = underside->copyIntersection(*paperMesh); // will be used to save what was on the paper before fold
    if (!check) { 
        std::cout << "Failed to copy underlayer" << std::endl; 
        return false; 
    }

    return true;
}

// rule of 5
Paper::Fold::~Fold() {
    delete underside; underside = nullptr;
    delete cover; cover = nullptr;
}

Paper::Fold::Fold(const Fold& other) :
    underside(other.underside ? new DyMesh(*other.underside) : nullptr),
    cover(other.cover ? new DyMesh(*other.cover) : nullptr),
    holds(other.holds),
    side(other.side)
{}

Paper::Fold::Fold(Fold&& other) noexcept :
    underside(other.underside),
    cover(other.cover),
    holds(std::move(other.holds)),
    side(other.side)
{
    other.underside = nullptr;
    other.cover = nullptr;
}

Paper::Fold& Paper::Fold::operator=(const Fold& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    Fold temp(other);
    
    delete underside;
    delete cover;
    
    underside = temp.underside;
    cover = temp.cover;
    holds = std::move(temp.holds);
    side = temp.side;
    
    temp.underside = nullptr;
    temp.cover = nullptr;
    
    return *this;
}

Paper::Fold& Paper::Fold::operator=(Fold&& other) noexcept {
    if (this == &other) return *this;
    
    delete underside;
    delete cover;
    
    underside = other.underside;
    cover = other.cover;
    holds = std::move(other.holds);
    side = other.side;
    
    other.underside = nullptr;
    other.cover = nullptr;
    
    return *this;
}