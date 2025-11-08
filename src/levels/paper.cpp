#include "levels/levels.h"

Paper::Paper() 
    : curSide(nullptr), isOpen(false) 
{
    sides = { nullptr, nullptr };
}

Paper::Paper(SingleSide* sideA, SingleSide* sideB, int startSide, bool isOpen) : isOpen(isOpen) {
    sides = { sideA, sideB };
    curSide = startSide == 0 ? sideA : sideB;
}

Paper::Paper(const Paper& other) noexcept
    : curSide(nullptr), isOpen(false)
{
    sides = { nullptr, nullptr };
    *this = other;
}

Paper::Paper(Paper&& other) noexcept
    : curSide(nullptr), isOpen(false)
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

    if (other.curSide == nullptr)
        curSide = nullptr;
    else
        curSide = (other.curSide == other.sides.first ? sides.first : sides.second);

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
    other.curSide = nullptr;
    other.isOpen = false;

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
    curSide = nullptr;
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
    folds.push_back(Fold(meshVertices, {0, 0}, 0));
}