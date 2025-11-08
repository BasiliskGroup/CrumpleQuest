#include "levels/levels.h"

Paper::Paper() 
    : curSide(0), isOpen(false) 
{
    sides = { nullptr, nullptr };
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
}

Paper::Paper(SingleSide* sideA, SingleSide* sideB, short startSide, bool isOpen) : isOpen(isOpen) {
    sides = { sideA, sideB };
    curSide = startSide;
}

Paper::Paper(const Paper& other) noexcept
    : curSide(0), isOpen(false)
{
    sides = { nullptr, nullptr };
    *this = other;
}

Paper::Paper(Paper&& other) noexcept
    : curSide(0), isOpen(false)
{
    sides = { nullptr, nullptr };
    *this = std::move(other);
}

Paper::~Paper() {
    clear();
}

Paper& Paper::operator=(const Paper& other) noexcept {
    if (this == &other) return *this;
    clear();

    if (other.sides.first)
        sides.first = new SingleSide(*other.sides.first);
    if (other.sides.second)
        sides.second = new SingleSide(*other.sides.second);

    curSide = other.curSide;

    isOpen = other.isOpen;
    return *this;
}

Paper& Paper::operator=(Paper&& other) noexcept {
    if (this == &other) return *this;
    clear();

    // transfer
    sides = std::move(other.sides);
    curSide = other.curSide;
    isOpen = other.isOpen;

    // clear other
    other.sides = { nullptr, nullptr };

    return *this;
}

void Paper::flip() {

}

void Paper::open() {

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

Paper::Fold::Fold(const std::vector<vec2>& verts, vec2 crease, int layer) 
    : crease(crease), layer(layer) 
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
    }
}

bool Paper::Fold::contains(const vec2& pos) {
    for (const Tri& tri : triangles) {
        if (tri.contains(pos)) return true;
    }
    return false;
}

void Paper::initFolds() {
    // folds.push_back(Fold(meshVerts, {0, 0}, 0));
}

Paper::PaperMesh::PaperMesh() : mesh(nullptr), verts() {}

Paper::PaperMesh::PaperMesh(const std::vector<Vert>& verts) : mesh(nullptr), verts(verts) {
    std::vector<float> data; 
    Paper::flattenVertices(verts, data);
    mesh = new Mesh(data);
}

Paper::PaperMesh::~PaperMesh() {
    delete mesh; mesh = nullptr;
}

Mesh* Paper::getMesh() { 
    PaperMesh* curMesh = curSide ? paperMeshes.second : paperMeshes.first; 
    if (curMesh == nullptr) return nullptr;
    return curMesh->mesh;
}