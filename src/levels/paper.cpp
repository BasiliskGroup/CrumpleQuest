#include "levels/levels.h"

Paper::PaperMesh::PaperMesh() : mesh(nullptr), verts() {}

Paper::PaperMesh::PaperMesh(const std::vector<Vert>& verts) : mesh(nullptr), verts(verts) {
    std::vector<float> data; 
    Paper::flattenVertices(verts, data);
    mesh = new Mesh(data);
}

Paper::PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
}

// Copy constructor
Paper::PaperMesh::PaperMesh(const PaperMesh& other) : verts(other.verts), mesh(nullptr) {
    if (other.mesh) {
        std::vector<float> data;
        Paper::flattenVertices(verts, data);
        mesh = new Mesh(data);
    }
}

// Move constructor
Paper::PaperMesh::PaperMesh(PaperMesh&& other) noexcept : verts(std::move(other.verts)), mesh(other.mesh) {
    other.mesh = nullptr;
}

// Copy assignment
Paper::PaperMesh& Paper::PaperMesh::operator=(const PaperMesh& other) {
    if (this == &other) return *this;
    
    delete mesh;
    mesh = nullptr;
    
    verts = other.verts;
    
    if (other.mesh) {
        std::vector<float> data;
        Paper::flattenVertices(verts, data);
        mesh = new Mesh(data);
    }
    
    return *this;
}

// Move assignment
Paper::PaperMesh& Paper::PaperMesh::operator=(PaperMesh&& other) noexcept {
    if (this == &other) return *this;
    
    delete mesh;
    
    verts = std::move(other.verts);
    mesh = other.mesh;
    other.mesh = nullptr;
    
    return *this;
}

// ==================== Paper Implementation ====================

Paper::Paper() 
    : curSide(0), isOpen(false) 
{
    sides = { nullptr, nullptr };
    paperMeshes = { nullptr, nullptr };
}

Paper::Paper(Mesh* mesh) : 
    curSide(0), 
    isOpen(false), 
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    const auto& meshVerts = mesh->getVertices();

    std::vector<Vert> verts;
    for (uint i = 0; i < meshVerts.size(); i += 5) {
        // no i + 2 since that is z and we're flat
        verts.push_back({ { meshVerts[i + 0], meshVerts[i + 1] }, { meshVerts[i + 2], meshVerts[i + 3] } });
    }
    paperMeshes.first = new PaperMesh(verts);

    // reverse x direction for other side of paper
    for (Vert& v : verts) {
        v.pos.x = -v.pos.x;
        // TODO check if we need to invert UVs
    }
    paperMeshes.second = new PaperMesh(verts);

    initFolds();
}

Paper::Paper(SingleSide* sideA, SingleSide* sideB, short startSide, bool isOpen) : isOpen(isOpen) {
    sides = { sideA, sideB };
    paperMeshes = { nullptr, nullptr };
    curSide = startSide;
}

// Copy constructor
Paper::Paper(const Paper& other) noexcept
    : curSide(other.curSide), 
      isOpen(other.isOpen),
      folds(other.folds),
      activeFold(other.activeFold)
{
    // Deep copy sides
    if (other.sides.first)
        sides.first = new SingleSide(*other.sides.first);
    else
        sides.first = nullptr;
        
    if (other.sides.second)
        sides.second = new SingleSide(*other.sides.second);
    else
        sides.second = nullptr;
    
    // Deep copy paperMeshes
    if (other.paperMeshes.first)
        paperMeshes.first = new PaperMesh(*other.paperMeshes.first);
    else
        paperMeshes.first = nullptr;
        
    if (other.paperMeshes.second)
        paperMeshes.second = new PaperMesh(*other.paperMeshes.second);
    else
        paperMeshes.second = nullptr;
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
    other.activeFold = -1;
}

Paper::~Paper() {
    clear();
}

// Copy assignment
Paper& Paper::operator=(const Paper& other) noexcept {
    if (this == &other) return *this;
    
    clear();

    // Copy sides
    if (other.sides.first)
        sides.first = new SingleSide(*other.sides.first);
    if (other.sides.second)
        sides.second = new SingleSide(*other.sides.second);

    // Copy paperMeshes
    if (other.paperMeshes.first)
        paperMeshes.first = new PaperMesh(*other.paperMeshes.first);
    if (other.paperMeshes.second)
        paperMeshes.second = new PaperMesh(*other.paperMeshes.second);

    curSide = other.curSide;
    isOpen = other.isOpen;
    folds = other.folds;
    activeFold = other.activeFold;
    
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
    other.activeFold = -1;

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
}

Paper::Fold::Fold(const std::vector<vec2>& verts, vec2 crease, int layer, int side) 
    : crease(crease), layer(layer), side(side)
{
    // find indices
    std::vector<uint> inds;
    Navmesh::earcut({verts}, inds);
    
    // create triangles
    for (uint i = 0; i < inds.size(); i += 3) {
        triangles.push_back(Tri({ 
            verts[inds[i + 0]],
            verts[inds[i + 1]],
            verts[inds[i + 2]]
        }));
        std::cout << "<" << verts[inds[i + 0]].x << ", " << verts[inds[i + 0]].y << ">";
        std::cout << "<" << verts[inds[i + 1]].x << ", " << verts[inds[i + 1]].y << ">";
        std::cout << "<" << verts[inds[i + 2]].x << ", " << verts[inds[i + 1]].y << ">";
        std::cout << std::endl;
    }
}

bool Paper::Fold::contains(const vec2& pos) {
    for (const Tri& tri : triangles) {
        if (tri.contains(pos)) return true;
    }
    return false;
}

void Paper::initFolds() {
    if (paperMeshes.first == nullptr) {
        throw std::runtime_error("Cannot create fold with no paper mesh");
    }

    std::vector<vec2> verts;
    for (const auto& v : paperMeshes.first->verts) {
        verts.push_back({ v.pos.x, v.pos.y });
    }

    folds.push_back(Fold(verts, {0, 0}, 0));
}

Mesh* Paper::getMesh() { 
    PaperMesh* curMesh = curSide ? paperMeshes.second : paperMeshes.first; 
    if (curMesh == nullptr) return nullptr;
    return curMesh->mesh;
}

void Paper::activateFold(const vec2& start) {
    activeFold = -1;

    // we push_back like a stack so the first fold we find is the top
    // FIXED: was i >= 0 which causes undefined behavior with unsigned int
    for (int i = static_cast<int>(folds.size()) - 1; i >= 0; i--) {
        if (folds[i].contains(start)) {
            activeFold = i;
            std::cout << i << std::endl;
            return;
        }
    }
    std::cout << -1 << std::endl;
}

void Paper::deactivateFold() {
    activeFold = -1;
}

void Paper::fold(const vec2& start, const vec2& end) {
    if (activeFold == -1 || glm::length2(start - end) < EPSILON) return;

    // 1. locate triangle that we are on
    // 2. find edge that we are grabbing
    // 3. get crease line
    // 4. create fold polygon
    // 5. clip polygon to remove folded over section
    // 6. earcut remaining paper and remap uvs
    // 7. earcut fold and mirror uvs
    // 8. clip the back (should be no uv changes)
    // 9. lock folds if they are folded back or covered. 
}