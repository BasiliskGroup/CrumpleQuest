#include "levels/levels.h"

PaperMesh::PaperMesh(const std::vector<vec2> verts, Mesh* mesh) : DyMesh(verts, mesh), mesh(nullptr) {
    std::vector<float> data; 
    toData(data);
    this->mesh = new Mesh(data);
}

PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
}

PaperMesh::PaperMesh(const PaperMesh& other) : DyMesh(other.region, other.regions), mesh(nullptr) {
    std::vector<float> data;
    toData(data);
    mesh = new Mesh(data);
}

PaperMesh::PaperMesh(PaperMesh&& other) noexcept : DyMesh(std::move(other.region), std::move(other.regions)), mesh(other.mesh) {
    other.mesh = nullptr;
}

PaperMesh& PaperMesh::operator=(const PaperMesh& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    PaperMesh temp(other);
    
    delete mesh;
    mesh = temp.mesh;
    region = std::move(temp.region);
    regions = std::move(temp.regions);
    temp.mesh = nullptr;
    
    return *this;
}

// Move assignment
PaperMesh& PaperMesh::operator=(PaperMesh&& other) noexcept {
    if (this == &other) return *this;
    
    delete mesh;
    
    region = std::move(other.region);
    regions = std::move(other.regions);
    mesh = other.mesh;
    other.mesh = nullptr;
    
    return *this;
}

void PaperMesh::regenerateMesh() {
    Mesh* oldPaperMesh = mesh;
    std::vector<float> newMeshData;
    toData(newMeshData);
    mesh = new Mesh(newMeshData);
    delete oldPaperMesh;
}